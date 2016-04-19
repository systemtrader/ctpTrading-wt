#include "TradeLogic.h"
#include "../../libs/Socket.h"
#include "../../cmd.h"
#include <vector>
#include <fstream>

TradeLogic::TradeLogic(int countMax, int countMin, int countMean, int kRang,
        int sellCloseKLineNum, int buyCloseKLineNum)
{
    _openMaxKLineNum  = countMax;
    _openMinKLineNum  = countMin;
    _openMeanKLineNum = countMean;
    _kRang = kRang;
    _sellCloseKLineNum = sellCloseKLineNum;
    _buyCloseKLineNum  = buyCloseKLineNum;

    _max = _min = _mean = 0;
    _store = new Redis("127.0.0.1", 6379, 1);

    _cfdIp   = getOptionToChar("trade_backend_srv_ip");
    _cfdPort = getOptionToInt("trade_backend_srv_port");

}

TradeLogic::~TradeLogic()
{
    delete _store;
}

void TradeLogic::init()
{
    string res;
    KLineBlock tmp;
    vector<string> params;
    while(1) {
        res = _store->pop("HISTORY_KLINE");
        if (res.length() == 0) break;
        params = Lib::split(res, "_");
        tmp = KLineBlock::makeSimple(params[0], params[8], params[3],
            params[5], params[6], params[4], params[7]);
        _bList.push_front(tmp);
    }
}

void TradeLogic::onKLineOpen()
{
    if ((int)_bList.size() == 0) return;
    int status = _getStatus();
    KLineBlock lastBlock = _bList.front();
    switch (status) {

        case TRADE_STATUS_NOTHING: // 没有任何操作，只能计算开仓条件
            _calculateOpen();
            _openedKLineMax = _openedKLineMin = lastBlock.getClosePrice(); // 初始化
            break;
        case TRADE_STATUS_BUYOPENING: // 状态为正在买开仓，则K线闭合时，可能买成功，也可能放弃本次购买重新开仓，所以计算卖平仓与开仓条件
            _calculateOpen();
            _calculateSellClose();
            _openedKLineMin = lastBlock.getClosePrice(); // 初始化
            break;
        case TRADE_STATUS_SELLOPENING: // 与买开仓类似
            _calculateOpen();
            _calculateBuyClose();
            _openedKLineMax = lastBlock.getClosePrice(); // 初始化
            break;

        case TRADE_STATUS_BUYOPENED: // 买开仓已成状态，计算卖平仓参数
            _calculateSellClose();
            _openedKLineMin = lastBlock.getClosePrice(); // 初始化
            break;
        case TRADE_STATUS_SELLCLOSING: // 正在卖平中，则K线闭合时，可能卖成，状态转变为空仓，计算开仓参数，也可能一直再卖，所以还要计算卖平条件
            _calculateOpen();
            _calculateSellClose();
            _openedKLineMin = lastBlock.getClosePrice(); // 初始化
            break;

        case TRADE_STATUS_SELLOPENED: // 与上述类似
            _calculateBuyClose();
            _openedKLineMax = lastBlock.getClosePrice(); // 初始化
            break;
        case TRADE_STATUS_BUYCLOSING: // 同上
            _calculateOpen();
            _calculateBuyClose();
            _openedKLineMax = lastBlock.getClosePrice(); // 初始化
            break;

        default:
            break;
    }
}

void TradeLogic::onKLineClose(KLineBlock block)
{
    _bList.push_front(block);
    int status = _getStatus();
    switch (status) {

        case TRADE_STATUS_NOTHING: // 空仓，判断是否开仓
        case TRADE_STATUS_BUYOPENING: // 状态为正在买开仓，说明从上一个K线关闭一直没买成功，则放弃重新买
        case TRADE_STATUS_SELLOPENING: // 同上
            if (block.getClosePrice() > _max) {
                _sendMsg(CMD_MSG_TRADE_BUYOPEN, block.getClosePrice());
            } else if (block.getClosePrice() < _min) {
                _sendMsg(CMD_MSG_TRADE_SELLOPEN, block.getClosePrice());
            } else { // 不符合开仓条件
                if (status == TRADE_STATUS_BUYOPENING || status == TRADE_STATUS_SELLOPENING)
                    _sendMsg(CMD_MSG_TRADE_CANCEL);
            }
            break;

        case TRADE_STATUS_BUYOPENED:
        case TRADE_STATUS_SELLCLOSING:
            if (block.getClosePrice() < _sellClosePoint) {
                Tick tick = _getTick();
                _sendMsg(CMD_MSG_TRADE_SELLCLOSE, tick.bidPrice1);
            } else {
                if (status == TRADE_STATUS_SELLCLOSING)
                    _sendMsg(CMD_MSG_TRADE_CANCEL);
            }
            break;

        case TRADE_STATUS_SELLOPENED:
        case TRADE_STATUS_BUYCLOSING:
            if (block.getClosePrice() > _buyClosePoint) {
                Tick tick = _getTick();
                _sendMsg(CMD_MSG_TRADE_BUYCLOSE, tick.askPrice1);
            } else {
                if (status == TRADE_STATUS_BUYCLOSING)
                    _sendMsg(CMD_MSG_TRADE_CANCEL);
            }
            break;

        default:
            break;
    }
}

int TradeLogic::_getStatus()
{
    string status = _store->get("TRADE_STATUS");
    return Lib::stoi(status);
}

void TradeLogic::_calculateOpen()
{
    // 获取最大K线个数（当前几个参数相同）
    int count = 0;
    count = _openMaxKLineNum > _openMinKLineNum ? _openMaxKLineNum : _openMinKLineNum;
    count = count > _openMeanKLineNum ? count : _openMeanKLineNum;

    // 当前K线不足判断，直接返回，不作操作
    if ((int)_bList.size() < count) return;

    // 计算当前范围内的开仓参数
    list<KLineBlock>::iterator item = _bList.begin();
    _max = item->getMaxPrice();
    _min = item->getMinPrice();

    double sum = 0; // 计算平均值用
    int cnt = count, currIndex = item->getIndex();
    while (1) {
        _max = _max > item->getMaxPrice() ? _max : item->getMaxPrice();
        _min = _min < item->getMinPrice() ? _min : item->getMinPrice();
        sum += item->getMinPrice();
        if (--cnt == 0) break;
        item++;
    }
    _mean = sum / count;

    // log
    ofstream info;
    Lib::initInfoLogHandle(info);
    info << "LogicFrontend[calculateOpen]" << "|";
    info << "max" << "|" << _max << "|";
    info << "mean" << "|" << _mean << "|";
    info << "min" << "|" << _min << "|";
    info << "index" << "|" << currIndex << endl;
    info.close();

}

void TradeLogic::_calculateBuyClose()
{
    KLineBlock lastBlock = _bList.front();
    _openedKLineMin = _openedKLineMin < lastBlock.getClosePrice() ? _openedKLineMin : lastBlock.getClosePrice();
    _buyClosePoint = _openedKLineMin + _kRang * (_buyCloseKLineNum - 0.5);

    //log
    ofstream info;
    Lib::initInfoLogHandle(info);
    info << "LogicFrontend[calculateBuyClose]" << "|";
    info << "openedKLineMin" << "|" << _openedKLineMin << "|";
    info << "buyClosePoint" << "|" << _buyClosePoint << endl;
    info.close();
}

void TradeLogic::_calculateSellClose()
{
    KLineBlock lastBlock = _bList.front();
    _openedKLineMax = _openedKLineMax > lastBlock.getClosePrice() ? _openedKLineMax : lastBlock.getClosePrice();
    _sellClosePoint = _openedKLineMax - _kRang * (_sellCloseKLineNum - 0.5);

    //log
    ofstream info;
    Lib::initInfoLogHandle(info);
    info << "LogicFrontend[calculateSellClose]" << "|";
    info << "openedKLineMax" << "|" << _openedKLineMax << "|";
    info << "sellClosePoint" << "|" << _sellClosePoint << endl;
    info.close();
}

Tick TradeLogic::_getTick()
{
    string tickStr = _store->get("CURRENT_TICK");
    vector<string> params = Lib::split(tickStr, "_");
    Tick tick = {0};
    tick.date   = params[1];
    tick.time   = params[2];
    tick.price  = Lib::stod(params[3]);
    tick.volume = Lib::stoi(params[4]);
    tick.bidPrice1 = Lib::stod(params[5]);
    tick.askPrice1 = Lib::stod(params[6]);
    return tick;
}

void TradeLogic::_sendMsg(string msg, double price)
{
    int cfd = getCSocket(_cfdIp, _cfdPort);
    string cmd = msg + "_" + Lib::dtos(price);
    sendMsg(cfd, cmd);
    close(cfd);

    //log
    KLineBlock lastBlock = _bList.front();
    ofstream info;
    Lib::initInfoLogHandle(info);
    info << "LogicFrontend[sendMsg]" << "|";
    info << "action" << "|" << msg << "|";
    info << "price" << "|" << price << "|";
    info << "kLineIndex" << "|" << lastBlock.getIndex() << endl;
    info.close();
}
