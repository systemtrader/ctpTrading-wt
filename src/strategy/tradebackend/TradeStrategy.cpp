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

void TradeStrategy::_clearTrade()
{
    if (_tradingOrderID.size() <= 0) return;
    list<int>::iterator i;
    for (i = _tradingOrderID.begin(); i != _tradingOrderID.end(); ++i) {
        _cancelAction(*i);
        _tradingOrderID.erase(i);
    }
}

void TradeStrategy::tradeAction(int action, double price, int total, int orderID)
{
    _clearTrade();
    int status = _getStatus();
    switch (action) {

        case TRADE_ACTION_BUYOPEN:
            _setStatus(TRADE_STATUS_BUYOPENING);
            _sendMsg(price, total, true, true, orderID);
            break;

        case TRADE_ACTION_SELLOPEN:
            _setStatus(TRADE_STATUS_SELLOPENING);
            _sendMsg(price, total, false, true, orderID);
            break;

        case TRADE_ACTION_BUYCLOSE:
            _setStatus(TRADE_STATUS_BUYCLOSING);
            _sendMsg(price, total, true, false, orderID);
            break;

        case TRADE_ACTION_SELLCLOSE:
            _setStatus(TRADE_STATUS_SELLCLOSING);
            _sendMsg(price, total, false, false, orderID);
            break;

        default:
            break;
    }
    _tradingOrderID.push_front(orderID);
    // 启动定时器
    setTimer(orderID);
}

void TradeStrategy::onTradeMsgBack(bool isSuccess, int orderID)
{
    if (isSuccess) {
        _successBack(orderID);
    } else {
        _cancelBack(orderID);
    }
}

bool TradeStrategy::_isInOrderIDList(list<int> l, int orderID)
{
    if (l.size() == 0) return false;
    list<int>::iterator i;
    for (i = l.begin(); i != l.end(); i++) {
        if (orderID == *i) return true;
    }
    return false;
}

list<int> TradeStrategy::_removeList(list<int> l, int orderID)
{
    if (l.size() == 0) return l;
    list<int>::iterator i;
    for (i = l.begin(); i != l.end(); i++) {
        if (orderID == *i) {
            l.erase(i);
            break;
        }
    }
    return l;
}

void TradeStrategy::_successBack(int orderID)
{
    _tradingOrderID = _removeList(_tradingOrderID, orderID);
    int status = _getStatus();
    switch (status) {
        case TRADE_STATUS_BUYOPENING:
            _setStatus(TRADE_STATUS_BUYOPENED);
            break;
        case TRADE_STATUS_SELLOPENING:
            _setStatus(TRADE_STATUS_SELLOPENED);
            break;
        case TRADE_STATUS_SELLCLOSING:
        case TRADE_STATUS_BUYCLOSING:
            _setStatus(TRADE_STATUS_NOTHING);
            break;
        default:
            break;
    }

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[successBack]";
    info << "|kIndex|" << orderID;
    info << "|status|" << _getStatus();
    info << endl;
    info.close();
}

void TradeStrategy::_cancelBack(int orderID)
{
    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[cancelBack]";
    info << "|kIndex|" << orderID;
    info << endl;
    info.close();

    if (_isInOrderIDList(_tradingOrderID, orderID)) {
        _zhuijia(orderID);
    }

}

void TradeStrategy::timeout(int orderID)
{

    if (_isInOrderIDList(_tradingOrderID, orderID)) {
        ofstream info;
        Lib::initInfoLogHandle(_logPath, info);
        info << "TradeStrategySrv[timeout]";
        info << "|kIndex|" << orderID;
        info << endl;
        info.close();
        _cancelAction(orderID);
    }

}

void TradeStrategy::_cancelAction(int orderID)
{
    MSG_TO_TRADE msg = {0};
    msg.msgType = MSG_ORDER_CANCEL;
    msg.orderID = orderID;
    _tradeSrvClient->send((void *)&msg);

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[cancel]";
    info << "|kIndex|" << orderID;
    info << endl;
    info.close();
}

void TradeStrategy::_zhuijia(int orderID)
{
    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[zhuijia]";
    info << "|kIndex|" << orderID;
    info << endl;
    info.close();

    double price;
    TickData tick = _getTick();
    int status = _getStatus();
    switch (status) {
        case TRADE_STATUS_SELLOPENING:
            price = tick.price;
            _sendMsg(price, 1, false, true, orderID);
            break;
        case TRADE_STATUS_BUYOPENING:
            price = tick.price;
            _sendMsg(price, 1, true, true, orderID);
            break;
        case TRADE_STATUS_SELLCLOSING:
            price = tick.price - 10;
            _sendMsg(price, 1, false, false, orderID);
            break;
        case TRADE_STATUS_BUYCLOSING:
            price = tick.price + 10;
            _sendMsg(price, 1, true, false, orderID);
            break;
        default:
            break;
    }
    // 启动定时器
    setTimer(orderID);
}

void TradeStrategy::_sendMsg(double price, int total, bool isBuy, bool isOpen, int orderID)
{
    MSG_TO_TRADE msg = {0};
    msg.msgType = MSG_ORDER;
    msg.price = price;
    msg.isBuy = isBuy;
    msg.total = total;
    msg.isOpen = isOpen;
    msg.orderID = orderID;
    _tradeSrvClient->send((void *)&msg);

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[sendOrder]";
    info << "|price|" << price;
    info << "|total|" << total;
    info << "|isBuy|" << isBuy;
    info << "|isOpen|" << isOpen;
    info << "|kIndex|" << orderID;
    info << endl;
    info.close();
}

TickData TradeStrategy::_getTick()
{
    string tickStr = _store->get("CURRENT_TICK");
    return Lib::string2TickData(tickStr);
}

int TradeStrategy::_getStatus()
{
    string status = _store->get("TRADE_STATUS");
    return Lib::stoi(status);
}

void TradeStrategy::_setStatus(int status)
{
    _store->set("TRADE_STATUS", Lib::itos(status));
}
