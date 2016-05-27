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
    // 设定定时器
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

TradeStrategy::TradeStrategy(int serviceID, string logPath, int db)
{
    _orderID = 0;
    _logPath = logPath;
    _store = new Redis("127.0.0.1", 6379, db);
    _tradeSrvClient = new QClient(serviceID, sizeof(MSG_TO_TRADE));

}

TradeStrategy::~TradeStrategy()
{
    // delete _store;
    // delete _tradeSrvClient;
    cout << "~TradeStrategy" << endl;
}

int TradeStrategy::_initTrade(int action, int kIndex, int total, string instrumnetID, double price)
{
    _orderID++;

    TRADE_DATA order = {0};
    order.action = action;
    order.price = price;
    order.kIndex = kIndex;
    order.total = total;
    order.instrumnetID = instrumnetID;
    _tradingInfo[_orderID] = order;

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[initTrade]";
    info << "|orderID|" << _orderID;
    info << "|iID|" << instrumnetID;
    info << "|kIndex|" << kIndex;
    info << endl;
    info.close();

    // save data
    string data = "klineorder_" + Lib::itos(kIndex) + "_" + Lib::itos(_orderID) + "_" + instrumnetID;
    _store->push("ORDER_LOGS", data);

    return _orderID;
}

void TradeStrategy::_clearTradeInfo(int orderID)
{
    std::map<int, TRADE_DATA>::iterator i = _tradingInfo.find(orderID);
    if (i != _tradingInfo.end()) _tradingInfo.erase(i);

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

void TradeStrategy::tradeAction(int action, double price, int total, int kIndex, string instrumnetID)
{
    int orderID = _initTrade(action, kIndex, total, instrumnetID, price);
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
            _setStatus(TRADE_STATUS_BUYCLOSING, instrumnetID);
            _sendMsg(price, total, true, false, orderID);
            break;

        case TRADE_ACTION_SELLCLOSE:
            _setStatus(TRADE_STATUS_SELLCLOSING, instrumnetID);
            _sendMsg(price, total, false, false, orderID);
            break;

        default:
            break;
    }
    setTimer(orderID);
}

void TradeStrategy::onSuccess(int orderID)
{
    if (!_isTrading(orderID)) return;
    TRADE_DATA order = _tradingInfo[orderID];

    _clearTradeInfo(orderID);
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

    // log
    int status = _getStatus(order.instrumnetID);
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[successBack]";
    info << "|iID|" << order.instrumnetID;
    info << "|kIndex|" << order.kIndex;
    info << "|orderID|" << orderID;
    info << "|status|" << status;
    info << endl;
    info.close();
}

void TradeStrategy::onCancel(int orderID)
{
    if (!_isTrading(orderID)) return;
    TRADE_DATA order = _tradingInfo[orderID];

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[cancelBack]";
    info << "|orderID|" << orderID;
    info << "|iID|" << order.instrumnetID;
    info << "|kIndex|" << order.kIndex;
    info << endl;
    info.close();

    if (order.action == TRADE_ACTION_BUYCLOSE || 
        order.action == TRADE_ACTION_SELLCLOSE) 
    {
        _zhuijia(orderID);
    } else {
        switch (order.action) {
            case TRADE_ACTION_BUYOPEN:
            case TRADE_ACTION_SELLOPEN:
                _setStatus(TRADE_STATUS_NOTHING, order.instrumnetID);
                break;
            case TRADE_ACTION_BUYCLOSE:
                _setStatus(TRADE_STATUS_SELLOPENED, order.instrumnetID);
                break;
            case TRADE_ACTION_SELLCLOSE:
                _setStatus(TRADE_STATUS_BUYOPENED, order.instrumnetID);
                break;
            default:
                break;
        }
    }
    _clearTradeInfo(orderID);

}

void TradeStrategy::onCancelErr(int orderID) 
{
    if (!_isTrading(orderID)) return;
    //
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

void TradeStrategy::_cancel(int orderID)
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

}

void TradeStrategy::_zhuijia(int orderID)
{
    if (!_isTrading(orderID)) return;
    TRADE_DATA order = _tradingInfo[orderID];

    int newOrderID = _initTrade(order.action, order.kIndex, order.total, order.instrumnetID, order.price);

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
            price = tick.price - 10;
            _sendMsg(price, 1, false, false, newOrderID);
            break;
        case TRADE_ACTION_BUYCLOSE:
            price = tick.price + 10;
            _sendMsg(price, 1, true, false, newOrderID);
            break;
        default:
            break;
    }
    // 启动定时器
    setTimer(newOrderID);
}

void TradeStrategy::_sendMsg(double price, int total, bool isBuy, bool isOpen, int orderID)
{
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
