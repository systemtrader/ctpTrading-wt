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

void TradeStrategy::tradeAction(int action, double price, int total, int orderID)
{
    // 有订单在处理，进入等待队列
    if (_tradingOrderID.size() > 0) {
        _waitingOrders.push_front(orderID);
        WAITING_DATA info = {0};
        info.action = action;
        info.price = price;
        info.orderID = orderID;
        info.total = total;
        _waitingOrdersInfo[orderID] = info;
        return;
    }

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
    _tradingOrders[orderID] = 5; // 撤销+追价共重试n次
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

void TradeStrategy::_successBack(int orderID)
{
    map<int, int>::iterator i;
    i = _tradingOrderID.find(orderID);
    if(i != _tradingOrderID.end())
        _tradingOrderID.erase(i);

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

    // 继续执行
    if (_waitingOrders.size() > 0) {
        // 获取等待orderID，并清空
        int waitingOrderID = _waitingOrders.back();
        _waitingOrders.pop_back();

        // 获取数据，清空并发送订单
        map<int ,WAITING_DATA>::iterator i;
        i = _waitingOrdersInfo.find(waitingOrderID);
        if(i != maplive.end()) {
            WAITING_DATA info = *i;
            _waitingOrdersInfo.erase(i);
            this->tradeAction(info.action, info.price, info.total, info.orderID);
        }
    }
}

void TradeStrategy::_clearTradingOrder(int orderID)
{
    map<int ,WAITING_DATA>::iterator i;
    i = _waitingOrdersInfo.find(waitingOrderID);
    if(i != maplive.end()) {
        _waitingOrdersInfo.erase(i);
    }
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

    if (_tradingOrderID[orderID] > 0) {
        _tradingOrderID[orderID]--;
        _zhuijia(orderID);
    } else {
        _clearTradingOrder(orderID);
    }

}

void TradeStrategy::timeout(int orderID)
{
    if (_tradingOrderID[orderID] > 0) {
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
    _tradingOrderID[orderID]++; // 自己撤单不计

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
