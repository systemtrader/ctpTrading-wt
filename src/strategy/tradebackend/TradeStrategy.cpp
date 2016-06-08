#include "TradeStrategy.h"

timer_t timer;
extern int timeoutSec;
extern TradeStrategy * service;

void timeout(union sigval v)
{
    service->timeout(v.sival_int);
    return;
}

void setTimer(int orderID)
{
    struct sigevent evp;
    struct itimerspec ts;

    memset(&evp, 0, sizeof(evp));
    evp.sigev_notify = SIGEV_THREAD;
    evp.sigev_value.sival_int = orderID;
    evp.sigev_notify_function = timeout;
    timer_create(CLOCK_REALTIME, &evp, &timer);

    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    ts.it_value.tv_sec = timeoutSec;
    ts.it_value.tv_nsec = 0;
    timer_settime(timer, 0, &ts, NULL);
}

TradeStrategy::TradeStrategy(int serviceID, string logPath, int db, int serviceIDK)
{
    _orderID = 0;
    _logPath = logPath;
    _store = new Redis("127.0.0.1", 6379, db);
    _tradeSrvClient = new QClient(serviceID, sizeof(MSG_TO_TRADE));
    _klineClient = new QClient(serviceIDK, sizeof(MSG_TO_KLINE));
}

TradeStrategy::~TradeStrategy()
{
    // delete _store;
    // delete _tradeSrvClient;
    cout << "~TradeStrategy" << endl;
}

int TradeStrategy::_initTrade(int action, int kIndex, int total, string instrumnetID, double price, int forecastID, bool isForecast, bool isMain, int beforeStatus, bool isZhuijia)
{
    if (_orderID == 0) {
        string idStr = _store->get("ORDER_ID_MAX_" + instrumnetID);
        if (idStr.length() > 0)
            _orderID = Lib::stoi(idStr);
    }
    _orderID++;

    TRADE_DATA order = {0};
    order.action = action;
    order.price = price;
    order.kIndex = kIndex;
    order.total = total;
    order.instrumnetID = instrumnetID;
    order.forecastID = forecastID;
    order.isForecast = isForecast;
    order.isMain = isMain;
    order.beforeStatus = beforeStatus;
    _tradingInfo[_orderID] = order;

    _forecastID2OrderID[forecastID] = _orderID;

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[initTrade]";
    info << "|orderID|" << _orderID;
    info << "|forecastID|" << forecastID;
    info << "|isMain|" << isMain;
    info << "|isForecast|" << isForecast;
    info << "|iID|" << instrumnetID;
    info << "|kIndex|" << kIndex;
    info << endl;
    info.close();

    // save data
    string data = "klineorder_" + Lib::itos(kIndex) + "_" + Lib::itos(_orderID) + "_" + instrumnetID + "_" + Lib::itos(isForecast)
                + "_" + Lib::itos(isMain) + "_" + Lib::itos(isZhuijia);
    _store->push("ORDER_LOGS", data);

    return _orderID;
}

void TradeStrategy::_clearTradeInfo(int orderID)
{
    std::map<int, TRADE_DATA>::iterator i = _tradingInfo.find(orderID);
    if (i != _tradingInfo.end()) {
        int forecastID = (i->second).forecastID;
        std::map<int, int>::iterator j = _forecastID2OrderID.find(forecastID);
        if (j != _forecastID2OrderID.end()) _forecastID2OrderID.erase(j);
        _tradingInfo.erase(i);
    }

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[removeTrade]";
    info << "|orderID|" << orderID;
    info << endl;
    info.close();
}

bool TradeStrategy::_isTrading(int orderID)
{
    std::map<int, TRADE_DATA>::iterator i;
    i = _tradingInfo.find(orderID);
    return i == _tradingInfo.end() ? false : true;
}

bool TradeStrategy::_isForecasting(int forecastID)
{
    std::map<int, int>::iterator i = _forecastID2OrderID.find(forecastID);
    return i == _forecastID2OrderID.end() ? false : true;
}

void TradeStrategy::trade(MSG_TO_TRADE_STRATEGY msg)
{
    int status = _getStatus(string(msg.instrumnetID));
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[tradeCome]";
    info << "|iID|" << msg.instrumnetID;
    info << "|kIndex|" << msg.kIndex;
    info << "|status|" << status;
    info << endl;
    info.close();

    // 撤单直接发送
    if (msg.msgType == MSG_TRADE_ROLLBACK) {
        if (!_isForecasting(msg.forecastID)) return;
        int orderID = _forecastID2OrderID[msg.forecastID];
        _cancel(orderID, 2);
        return;
    }

    // 非主线单、预测单，直接发送
    if (!msg.isMain || msg.isForecast) {
        _tradeAction(msg);
        return;
    }

    // 剩下的请求，需要串行处理，所以处理中的状态要讲请求入队列
    if (status == TRADE_STATUS_BUYOPENING ||
        status == TRADE_STATUS_BUYCLOSING ||
        status == TRADE_STATUS_SELLOPENING ||
        status == TRADE_STATUS_SELLCLOSING)
    {
        _waitList.push_back(msg);
        return;
    }
    _tradeAction(msg);
}

void TradeStrategy::_tradeAction(MSG_TO_TRADE_STRATEGY msg)
{
    int action;
    double price = msg.price;
    int total = msg.total;
    int kIndex = msg.kIndex;
    string instrumnetID = string(msg.instrumnetID);
    int forecastID = msg.forecastID;

    if (msg.msgType == MSG_TRADE_BUYOPEN) {
        action = TRADE_ACTION_BUYOPEN;
    }
    if (msg.msgType == MSG_TRADE_SELLOPEN) {
        action = TRADE_ACTION_SELLOPEN;
    }
    if (msg.msgType == MSG_TRADE_SELLCLOSE) {
        action = TRADE_ACTION_SELLCLOSE;
    }
    if (msg.msgType == MSG_TRADE_BUYCLOSE) {
        action = TRADE_ACTION_BUYCLOSE;
    }
    int status = _getStatus(instrumnetID);
    int orderID = _initTrade(action, kIndex, total, instrumnetID, price, forecastID, msg.isForecast, msg.isMain, status);
    switch (action) {

        case TRADE_ACTION_BUYOPEN:
            _setStatus(TRADE_STATUS_BUYOPENING, instrumnetID);
            _sendMsg(price, total, true, true, orderID);
            break;

        case TRADE_ACTION_SELLOPEN:
            _setStatus(TRADE_STATUS_SELLOPENING, instrumnetID);
            _sendMsg(price, total, false, true, orderID);
            break;

        case TRADE_ACTION_BUYCLOSE:
            if (msg.isMain)
                _setStatus(TRADE_STATUS_BUYCLOSING, instrumnetID);
            else
                _setSecondStatus(TRADE_STATUS_BUYCLOSING, instrumnetID);
            _sendMsg(price, total, true, false, orderID);
            break;

        case TRADE_ACTION_SELLCLOSE:
            if (msg.isMain)
                _setStatus(TRADE_STATUS_SELLCLOSING, instrumnetID);
            else
                _setSecondStatus(TRADE_STATUS_SELLCLOSING, instrumnetID);
            _sendMsg(price, total, false, false, orderID);
            break;

        default:
            break;
    }
    if (!msg.isForecast)
        setTimer(orderID);
}

void TradeStrategy::onSuccess(int orderID, double price)
{
    if (!_isTrading(orderID)) return;
    TRADE_DATA order = _tradingInfo[orderID];

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[successBack]";
    info << "|orderID|" << orderID;
    info << "|isMain|" << order.isMain;
    info << "|isForecast|" << order.isForecast;
    info << "|waitingSize|" << _waitList.size();
    info << "|iID|" << order.instrumnetID;
    info << endl;
    info.close();

    _clearTradeInfo(orderID);

    // 只有主线单有机会更改状态
    if (order.isMain) {
        if (_waitList.size() == 0) {

            switch (order.action) {
                case TRADE_ACTION_BUYOPEN:
                    _setStatus(TRADE_STATUS_BUYOPENED, order.instrumnetID);
                    break;
                case TRADE_ACTION_SELLOPEN:
                    _setStatus(TRADE_STATUS_SELLOPENED, order.instrumnetID);
                    break;
                case TRADE_ACTION_BUYCLOSE:
                case TRADE_ACTION_SELLCLOSE:
                    _setStatus(TRADE_STATUS_NOTHING, order.instrumnetID);
                    break;
                default:
                    break;
            }

            if (order.isForecast) {
                // 生成一个Tick，发送给K线系统
                MSG_TO_KLINE msg = {0};
                msg.msgType = MSG_TICK;
                msg.tick.price = price;
                msg.tick.bidPrice1 = price;
                msg.tick.askPrice1 = price;
                strcpy(msg.tick.instrumnetID, order.instrumnetID.c_str());
                _klineClient->send((void *)&msg);

                // 将数据放入队列，以便存入DB
                string tickStr = Lib::tickData2String(msg.tick);
                string keyQ = "MARKET_TICK_Q";
                string keyD = "CURRENT_TICK_" + string(order.instrumnetID);
                _store->set(keyD, tickStr); // tick数据，供全局使用
                _store->push(keyQ, tickStr);
            }

        } else {

            MSG_TO_TRADE_STRATEGY msg = _waitList.front();
            _waitList.pop_front();
            _tradeAction(msg);

        }
    } else {
        switch (order.action) {

            case TRADE_ACTION_BUYCLOSE:
            case TRADE_ACTION_SELLCLOSE:
                _setSecondStatus(TRADE_STATUS_NOTHING, order.instrumnetID);
                break;
            default:
                break;
        }
    }
}

void TradeStrategy::onCancel(int orderID)
{
    if (!_isTrading(orderID)) return;
    TRADE_DATA order = _tradingInfo[orderID];
    int status = _getStatus(order.instrumnetID);
    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[cancelBack]";
    info << "|orderID|" << orderID;
    info << "|isMain|" << order.isMain;
    info << "|isForecast|" << order.isForecast;
    info << "|waitingSize|" << _waitList.size();
    info << "|iID|" << order.instrumnetID;
    info << endl;
    info.close();

    // 非预测单的平仓需要追价
    if ((order.action == TRADE_ACTION_SELLCLOSE ||
         order.action == TRADE_ACTION_BUYCLOSE) &&
        !order.isForecast)
    {
        _zhuijia(orderID);
        _clearTradeInfo(orderID);
        return;
    }

    // 主线单有机会改状态
    if (order.isMain) {
        if (_waitList.size() == 0) {
            switch (order.action) {
                case TRADE_ACTION_BUYOPEN:
                case TRADE_ACTION_SELLOPEN:
                    if (status == TRADE_STATUS_BUYOPENING || status == TRADE_STATUS_SELLOPENING)
                        _setStatus(order.beforeStatus, order.instrumnetID);
                    break;
                case TRADE_ACTION_BUYCLOSE:
                    if (status == TRADE_STATUS_BUYCLOSING)
                        _setStatus(order.beforeStatus, order.instrumnetID);
                    break;
                case TRADE_ACTION_SELLCLOSE:
                    if (status == TRADE_STATUS_SELLCLOSING)
                        _setStatus(order.beforeStatus, order.instrumnetID);
                    break;
                default:
                    break;
            }
        } else {
            MSG_TO_TRADE_STRATEGY msg = _waitList.front();
            _waitList.pop_front();
            _tradeAction(msg);
        }
    }
    _clearTradeInfo(orderID);

}

void TradeStrategy::onErr(int orderID, int errNo)
{
    if (!_isTrading(orderID)) return;
    //
    TRADE_DATA order = _tradingInfo[orderID];
    if (errNo == 50) {
        _clearTradeInfo(orderID);
        _setStatus(TRADE_STATUS_NOTHING, order.instrumnetID);
    }
}

void TradeStrategy::timeout(int orderID)
{
    if (!_isTrading(orderID)) return;
    TRADE_DATA order = _tradingInfo[orderID];

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[timeout]";
    info << "|iID|" << order.instrumnetID;
    info << "|kIndex|" << order.kIndex;
    info << "|orderID|" << orderID;
    info << endl;
    info.close();
    _cancel(orderID);

}

void TradeStrategy::_cancel(int orderID, int type)
{
    if (!_isTrading(orderID)) return;
    TRADE_DATA order = _tradingInfo[orderID];

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[cancel]";
    info << "|orderID|" << orderID;
    info << "|iID|" << order.instrumnetID;
    info << "|kIndex|" << order.kIndex;
    info << endl;
    info.close();

    MSG_TO_TRADE msg = {0};
    msg.msgType = MSG_ORDER_CANCEL;
    msg.orderID = orderID;
    _tradeSrvClient->send((void *)&msg);

    // save data
    string data = "klineordercancel_" + Lib::itos(orderID) + "_" + order.instrumnetID + "_" + Lib::itos(order.kIndex) + "_" + Lib::itos(type);
    _store->push("ORDER_LOGS", data);
}

void TradeStrategy::_zhuijia(int orderID)
{
    if (!_isTrading(orderID)) return;
    TRADE_DATA order = _tradingInfo[orderID];

    int newOrderID = _initTrade(order.action, order.kIndex, order.total, order.instrumnetID, order.price, order.forecastID, order.isForecast, order.isMain, order.beforeStatus, true);

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[zhuijia]";
    info << "|orderID|" << orderID;
    info << "|newOrderID|" << newOrderID;
    info << "|iID|" << order.instrumnetID;
    info << "|kIndex|" << order.kIndex;
    info << endl;
    info.close();

    double price;
    TickData tick = _getTick(order.instrumnetID);
    switch (order.action) {
        case TRADE_ACTION_SELLCLOSE:
            price = tick.bidPrice1;
            _sendMsg(price, 1, false, false, newOrderID);
            break;
        case TRADE_ACTION_BUYCLOSE:
            price = tick.askPrice1;
            _sendMsg(price, 1, true, false, newOrderID);
            break;
        default:
            break;
    }
    setTimer(newOrderID);
}

void TradeStrategy::_sendMsg(double price, int total, bool isBuy, bool isOpen, int orderID)
{
    usleep(1000);
    TRADE_DATA order = _tradingInfo[orderID];

    MSG_TO_TRADE msg = {0};
    msg.msgType = MSG_ORDER;
    msg.price = price;
    msg.isBuy = isBuy;
    msg.total = total;
    msg.isOpen = isOpen;
    msg.orderID = orderID;
    strcpy(msg.instrumnetID, Lib::stoc(order.instrumnetID));
    _tradeSrvClient->send((void *)&msg);

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[sendOrder]";
    info << "|iID|" << order.instrumnetID;
    info << "|price|" << price;
    info << "|total|" << total;
    info << "|isBuy|" << isBuy;
    info << "|isOpen|" << isOpen;
    info << "|kIndex|" << order.kIndex;
    info << "|orderID|" << orderID;
    info << endl;
    info.close();
}

TickData TradeStrategy::_getTick(string iID)
{
    string tickStr = _store->get("CURRENT_TICK_" + iID);
    return Lib::string2TickData(tickStr);
}

int TradeStrategy::_getStatus(string instrumnetID)
{
    string status = _store->get("TRADE_STATUS_" + instrumnetID);
    return Lib::stoi(status);
}

void TradeStrategy::_setStatus(int status, string instrumnetID)
{
    _store->set("TRADE_STATUS_" + instrumnetID, Lib::itos(status));
}

void TradeStrategy::_setSecondStatus(int status, string instrumnetID)
{
    _store->set("TRADE_SECOND_STATUS_" + instrumnetID, Lib::itos(status));
}
