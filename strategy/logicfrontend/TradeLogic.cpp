#include "TradeLogic.h"
#include <vector>
#include <fstream>

TradeLogic::TradeLogic(int countMax, int countMin, int countMean, int kRang, 
        int sellCloseKLineNum, int buyCloseKLineNum)
{
    _openMaxKLineNum = countMax;
    _openMinKLineNum = countMin;
    _openMeanKLineNum = countMean;
    _kRang = kRang;
    _sellCloseKLineNum = sellCloseKLineNum;
    _buyCloseKLineNum = buyCloseKLineNum;

    _max = _min = _mean = 0;
    _store = new Redis("127.0.0.1", 6379, 1);

    _store->set("TRADE_STATUS", Lib::itos(TRADE_STATUS_NOTHING)); // 模拟代码，要删掉TODO
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
    // cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    // cout << "status: " << status << endl;
    // cout << "closeAction: " << _closeAction << endl;
    // cout << "blist'length: " << _bList.size() << endl;
    // cout << "openIndex: " << _openIndex << endl;
    // cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
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
    // cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    // cout << "closeAction: " << _closeAction << endl;
    // cout << "blist'length: " << _bList.size() << endl;
    // 
    int status = _getStatus();
    switch (status) {

        case TRADE_STATUS_NOTHING: // 空仓，判断是否开仓
        case TRADE_STATUS_BUYOPENING: // 状态为正在买开仓，说明从上一个K线关闭一直没买成功，则放弃重新买
        case TRADE_STATUS_SELLOPENING: // 同上
            if (block.getClosePrice() > _max) {
                // 发送消息，购买系统中更新状态，现在模拟成功状态 TODO
                _store->set("TRADE_STATUS", Lib::itos(TRADE_STATUS_BUYOPENED));
                // log
                ofstream info;
                Lib::initInfoLogHandle(info);
                info << "onKLineClose" << "|";
                info << "BUY_OPEN" << "|";
                info << "OPEN_PRICE" << "|" << block.getClosePrice() << endl;
                info.close();
            } else if (block.getClosePrice() < _min) {
                // 发送消息，购买系统中更新状态，现在模拟成功状态 TODO
                _store->set("TRADE_STATUS", Lib::itos(TRADE_STATUS_SELLOPENED));
                // log
                ofstream info;
                Lib::initInfoLogHandle(info);
                info << "onKLineClose" << "|";
                info << "SELL_OPEN" << "|";
                info << "OPEN_PRICE" << "|" << block.getClosePrice() << endl;
                info.close();
            } else { // 不符合开仓条件
                // 通知下单系统放弃正在进行的操作 TODO
                _store->set("TRADE_STATUS", Lib::itos(TRADE_STATUS_NOTHING));
            }
            break;

        case TRADE_STATUS_BUYOPENED: 
        case TRADE_STATUS_SELLCLOSING: 
            if (block.getClosePrice() < _sellClosePoint) {
                // 发送消息，购买系统中更新状态，现在模拟成功状态 TODO
                _store->set("TRADE_STATUS", Lib::itos(TRADE_STATUS_NOTHING));
                // log
                ofstream info;
                Lib::initInfoLogHandle(info);
                info << "onKLineClose" << "|";
                info << "SELL_CLOSE" << "|";
                info << "CLOSE_PRICE" << "|" << block.getClosePrice() << endl;
                info.close();
            } else {
                _store->set("TRADE_STATUS", Lib::itos(TRADE_STATUS_BUYOPENED));
            }
            break;

        case TRADE_STATUS_SELLOPENED:
        case TRADE_STATUS_BUYCLOSING: 
            if (block.getClosePrice() > _buyClosePoint) {
                // 发送消息，购买系统中更新状态，现在模拟成功状态 TODO
                _store->set("TRADE_STATUS", Lib::itos(TRADE_STATUS_NOTHING));
                // log
                ofstream info;
                Lib::initInfoLogHandle(info);
                info << "onKLineClose" << "|";
                info << "BUY_CLOSE" << "|";
                info << "CLOSE_PRICE" << "|" << block.getClosePrice() << endl;
                info.close();
            } else {
                _store->set("TRADE_STATUS", Lib::itos(TRADE_STATUS_SELLOPENED));
            }
            break;

        default:
            break;
    }


    // cout << "openIndex: " << _openIndex << endl;
    // cout << "status: " << _getStatus() << endl;
    // cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;

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

