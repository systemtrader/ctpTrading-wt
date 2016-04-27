#include "TradeLogic.h"

TradeLogic::TradeLogic(int countMax, int countMin, int countMean, int kRang,
        int sellCloseKLineNum, int buyCloseKLineNum, int serviceID, string logPath, int isHistoryBack, int db)
{
    _openMaxKLineCount  = countMax;
    _openMinKLineCount  = countMin;
    _openMeanKLineCount = countMean;
    _kRang = kRang;
    _closeSellKRangeCount = sellCloseKLineNum;
    _closeBuyKRangeCount  = buyCloseKLineNum;
    _logPath = logPath;
    _isHistoryBack = isHistoryBack;

    _max = _min = _mean = 0;
    _store = new Redis("127.0.0.1", 6379, db);

    _tradeStrategySrvClient = new QClient(serviceID, sizeof(MSG_TO_TRADE_STRATEGY));

}

TradeLogic::~TradeLogic()
{
    // delete _store;
    // delete _tradeStrategySrvClient;
    cout << "~TradeLogicSrv" << endl;
}

void TradeLogic::init()
{
    string res;
    KLineBlock tmp;
    KLineBlockData tmpData = {0};
    vector<string> params;

    while(1) {
        res = _store->pop("HISTORY_KLINE");
        if (res.length() == 0) break;
        params = Lib::split(res, "_");
        tmpData.index = Lib::stoi(params[0]);
        tmpData.open = Lib::stod(params[5]);
        tmpData.max = Lib::stod(params[7]);
        tmpData.min = Lib::stod(params[8]);
        tmpData.close = Lib::stod(params[6]);
        tmp = KLineBlock::makeViaData(tmpData);
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
            if (_max > 0 && block.getClosePrice() > _max) {
                _sendMsg(MSG_TRADE_BUYOPEN, block.getClosePrice() - 10);
            } else if (_min > 0 && block.getClosePrice() < _min) {
                _sendMsg(MSG_TRADE_SELLOPEN, block.getClosePrice() + 10);
            } else { // 不符合开仓条件
                if (status == TRADE_STATUS_BUYOPENING || status == TRADE_STATUS_SELLOPENING)
                    _sendMsg(MSG_TRADE_CANCEL);
            }
            break;

        case TRADE_STATUS_BUYOPENED:
        case TRADE_STATUS_SELLCLOSING:
            if (_sellClosePoint > 0 && block.getClosePrice() < _sellClosePoint) {
                if (_isHistoryBack) {
                    _sendMsg(MSG_TRADE_SELLCLOSE, block.getClosePrice());
                } else {
                    Tick tick = _getTick();
                    _sendMsg(MSG_TRADE_SELLCLOSE, tick.bidPrice1);
                }
            } else {
                if (status == TRADE_STATUS_SELLCLOSING)
                    _sendMsg(MSG_TRADE_CANCEL);
            }
            break;

        case TRADE_STATUS_SELLOPENED:
        case TRADE_STATUS_BUYCLOSING:
            if (_buyClosePoint > 0 && block.getClosePrice() > _buyClosePoint) {
                if (_isHistoryBack) {
                    _sendMsg(MSG_TRADE_BUYCLOSE, block.getClosePrice());
                } else {
                    Tick tick = _getTick();
                    _sendMsg(MSG_TRADE_BUYCLOSE, tick.askPrice1);
                }
            } else {
                if (status == TRADE_STATUS_BUYCLOSING)
                    _sendMsg(MSG_TRADE_CANCEL);
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
    count = _openMaxKLineCount > _openMinKLineCount ? _openMaxKLineCount : _openMinKLineCount;
    count = count > _openMeanKLineCount ? count : _openMeanKLineCount;

    // 当前K线不足判断，直接返回，不作操作
    if ((int)_bList.size() < count) return;
    // count = count > (int)_bList.size() ? (int)_bList.size() : count;
    // 计算当前范围内的开仓参数
    list<KLineBlock>::iterator item = _bList.begin();
    // _max = item->getMaxPrice();
    // _min = item->getMinPrice();
    _max = item->getClosePrice();
    _min = item->getClosePrice();

    double sum = 0; // 计算平均值用
    int cnt = count, currIndex = item->getIndex();
    while (1) {
        // _max = _max > item->getMaxPrice() ? _max : item->getMaxPrice();
        // _min = _min < item->getMinPrice() ? _min : item->getMinPrice();
        _max = _max > item->getClosePrice() ? _max : item->getClosePrice();
        _min = _min < item->getClosePrice() ? _min : item->getClosePrice();

        // sum += item->getMinPrice();
        sum += item->getClosePrice();
        if (--cnt == 0) break;
        item++;
    }
    _mean = sum / count;

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeLogicSrv[calculateOpen]";
    info << "|max|" << _max;
    info << "|mean|" << _mean;
    info << "|min|" << _min;
    info << "|kIndex|" << currIndex << endl;
    info.close();

}

void TradeLogic::_calculateBuyClose()
{
    KLineBlock lastBlock = _bList.front();
    _openedKLineMin = _openedKLineMin < lastBlock.getClosePrice() ? _openedKLineMin : lastBlock.getClosePrice();
    _buyClosePoint = _openedKLineMin + _kRang * (_closeBuyKRangeCount - 0.5);

    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeLogicSrv[calculateBuyClose]";
    info << "|openedKLineMin|" << _openedKLineMin;
    info << "|buyClosePoint|" << _buyClosePoint;
    info << "|kIndex|" << lastBlock.getIndex();
    info << endl;
    info.close();
}

void TradeLogic::_calculateSellClose()
{
    KLineBlock lastBlock = _bList.front();
    _openedKLineMax = _openedKLineMax > lastBlock.getClosePrice() ? _openedKLineMax : lastBlock.getClosePrice();
    _sellClosePoint = _openedKLineMax - _kRang * (_closeSellKRangeCount - 0.5);

    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeLogicSrv[calculateSellClose]";
    info << "|openedKLineMax|" << _openedKLineMax;
    info << "|sellClosePoint|" << _sellClosePoint;
    info << "|kIndex|" << lastBlock.getIndex();
    info << endl;
    info.close();
}

TickData TradeLogic::_getTick()
{
    string tickStr = _store->get("CURRENT_TICK");
    return Lib::string2TickData(tickStr);
}

void TradeLogic::_sendMsg(int msgType, double price)
{
    KLineBlock lastBlock = _bList.front();
    MSG_TO_TRADE_STRATEGY msg = {0};
    msg.msgType = msgType;
    msg.price = price;
    msg.kIndex = lastBlock.getIndex();
    _tradeStrategySrvClient->send((void *)&msg);

    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeLogicSrv[sendMsg]";
    info << "|action|" << msgType;
    info << "|price|" << price;
    info << "|kIndex|" << lastBlock.getIndex() << endl;
    info.close();
}
