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
    ts.it_value.tv_sec = 0;
    ts.it_value.tv_nsec = timeoutSec * 1000 * 1000;
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

void TradeStrategy::accessAction(MSG_TO_TRADE_STRATEGY msg)
{
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[accessAction]";
    info << "|msgType|" << msg.msgType;

    if (msg.msgType == MSG_TRADE_ROLLBACK) {
        info << endl;
        info.close();
        _rollback(msg.groupID);
        return;
    }
    if (msg.msgType == MSG_TRADE_FORECAST_OVER) {
        info << endl;
        info.close();
        _dealForecast();
        return;
    }
    if (msg.msgType == MSG_TRADE_REAL_COME) {
        info << endl;
        info.close();
        _dealRealCome();
        return;
    }
    // log
    info << "|iID|" << msg.instrumnetID;
    info << "|kIndex|" << msg.kIndex;
    info << "|groupID|" << msg.groupID;
    info << "|price|" << msg.price;
    info << endl;
    info.close();

    _waitingList.push_back(msg);
}


void TradeStrategy::onSuccess(int orderID)
{
    ORDER_DATA order = _allOrders[orderID];
    // log
    int status = _getStatus(order.instrumnetID);
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[onSuccess]";
    info << "|iID|" << order.instrumnetID;
    info << "|kIndex|" << order.kIndex;
    info << "|orderID|" << orderID;
    info << "|status|" << status;
    info << endl;
    info.close();

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
    _clearTrade(orderID);


    // 生成一个Tick，发送给K线系统
    MSG_TO_KLINE msg = {0};
    msg.msgType = MSG_TICK;
    msg.tick.price = order.price;
    msg.tick.bidPrice1 = order.price;
    msg.tick.askPrice1 = order.price;
    strcpy(msg.tick.instrumnetID, order.instrumnetID);
    _klineClient->send((void *)&msg);

    _findOrder2Send();

    // 将数据放入队列，以便存入DB
    string tickStr = Lib::tickData2String(msg.tick);
    string keyQ = "MARKET_TICK_Q";
    string keyD = "CURRENT_TICK_" + string(order.instrumnetID);
    _store->set(keyD, tickStr); // tick数据，供全局使用
    _store->push(keyQ, tickStr);
}

void TradeStrategy::onCancel(int orderID)
{
    ORDER_DATA order = _allOrders[orderID];
    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[onCancel]";
    info << "|iID|" << order.instrumnetID;
    info << "|kIndex|" << order.kIndex;
    info << "|orderID|" << orderID;
    info << "|groupID|" << order.groupID;
    info << endl;
    info.close();

    int status = _getStatus(order.instrumnetID);
    if (status == TRADE_STATUS_OPENING) {
        _setStatus(TRADE_STATUS_NOTHING, order.instrumnetID);
    }

    _clearTrade(orderID);
    _findOrder2Send();
}

void TradeStrategy::onCancelErr(int orderID)
{
    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[onCancelErr]";
    info << "|orderID|" << orderID;
    info << endl;
    info.close();

    _clearTrade(orderID);
    _findOrder2Send();
}

void TradeStrategy::timeout(int orderID)
{
    ORDER_DATA order = _allOrders[orderID];

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[timeout]";
    info << "|iID|" << order.instrumnetID;
    info << "|kIndex|" << order.kIndex;
    info << "|orderID|" << orderID;
    info << "|groupID|" << order.groupID;
    info << endl;
    info.close();

    _cancelOrderIDList.push_back(orderID);
    _zhuijia(order);
    _findOrder2Send();

}

void TradeStrategy::_dealForecast()
{
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[dealForecast]";
    info << endl;
    info.close();

    std::vector<MSG_TO_TRADE_STRATEGY>::iterator it;
    for (it = _waitingList.begin(); it != _waitingList.end(); it++) {
        _initTrade(*it);
    }
    _waitingList.clear();
    _findOrder2Send(true);
}

void TradeStrategy::_dealRealCome()
{
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[dealRealCome]";
    info << endl;
    info.close();

    int orderID;
    std::vector<MSG_TO_TRADE_STRATEGY>::iterator it;
    for (it = _waitingList.begin(); it != _waitingList.end(); it++) {
        _initTrade(*it, true);
    }
    _waitingList.clear();
    _findOrder2Send();
}

void TradeStrategy::_rollback(int groupID)
{
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[rollback]";
    info << "|orderID|";
    std::vector<int> orderIDs = _gid2OrderIDs[groupID];
    std::vector<int>::iterator i;
    for (i = orderIDs.begin(); i != orderIDs.end(); i++) {
        info << *i << ",";
        _cancelOrderIDList.push_back(*i);
    }
    info << endl;
    info.close();
}


void TradeStrategy::_zhuijia(ORDER_DATA data)
{
    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[zhuijia]";
    info << "|iID|" << data.instrumnetID;
    info << "|kIndex|" << data.kIndex;
    info << "|dataID|" << data.orderID;
    info << "|groupID|" << data.groupID;
    info << endl;
    info.close();

    TickData tick = _getTick(data.instrumnetID);
    MSG_TO_TRADE_STRATEGY msg = data.msg;
    switch (data.action) {
        case TRADE_ACTION_SELLCLOSE:
            msg.price = tick.bidPrice1;
            break;
        case TRADE_ACTION_BUYCLOSE:
            msg.price = tick.askPrice1;
            break;
        default:
            break;
    }
    _initTrade(data.msg, true);
}

void TradeStrategy::_cancel(int orderID)
{
    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[cancel]";
    info << "|orderID|" << orderID;
    info << endl;
    info.close();

    MSG_TO_TRADE msg = {0};
    msg.msgType = MSG_ORDER_CANCEL;
    msg.orderID = orderID;
    _tradeSrvClient->send((void *)&msg);
}

void TradeStrategy::_open(ORDER_DATA data)
{
    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[open]";
    info << "|orderID|" << data.orderID;
    info << endl;
    info.close();

    _setStatus(TRADE_STATUS_OPENING, string(data.instrumnetID));

    if (data.action == TRADE_ACTION_BUYOPEN) {
        _sendMsg(data.price, data.total, true, true, data.orderID);
    }
    if (data.action == TRADE_ACTION_SELLOPEN) {
        _sendMsg(data.price, data.total, false, true, data.orderID);
    }
}

void TradeStrategy::_close(ORDER_DATA data)
{
    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[close]";
    info << "|orderID|" << data.orderID;
    info << endl;
    info.close();

    if (data.action == TRADE_ACTION_BUYCLOSE) {
        _setStatus(TRADE_STATUS_BUYCLOSING, string(data.instrumnetID));
        _sendMsg(data.price, data.total, true, false, data.orderID);
    }
    if (data.action == TRADE_ACTION_SELLCLOSE) {
        _setStatus(TRADE_STATUS_SELLCOLSING, string(data.instrumnetID));
        _sendMsg(data.price, data.total, false, false, data.orderID);
    }
}

void TradeStrategy::_findOrder2Send(bool sendOpen)
{
        // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[findOrder2Send]";

    if (_cancelOrderIDList.size() > 0) {
        int cancelID = *(_cancelOrderIDList.begin());
        info << "|cancelID|" << cancelID;
        info << endl;
        info.close();
        _cancel(cancelID);
        return;
    }
    ORDER_DATA data;
    if (_closeOrderIDList.size() > 0) {
        int closeID = *(_closeOrderIDList.begin());
        info << "|closeID|" << closeID;
        info << endl;
        info.close();
        data = _allOrders[closeID];
        _close(data);
        if (data.setTimer)
            setTimer(data.orderID);
        return;
    }
    if (sendOpen && _openOrderIDList.size() > 0) {
        std::vector<int>::iterator i;
        info << "|openID|";
        for (i = _openOrderIDList.begin(); i != _openOrderIDList.end(); ++i)
        {
            info << *i << ",";
            data = _allOrders[(*i)];
            _open(data);
            if (data.setTimer)
                setTimer(data.orderID);

        }
        info << endl;
        info.close();
        return;
    }
    info << endl;
    info.close();
}

void TradeStrategy::_initTrade(MSG_TO_TRADE_STRATEGY msg, bool setTimer)
{
    _orderID++;

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[initTrade]";
    info << "|orderID|" << _orderID;
    info << endl;
    info.close();

    ORDER_DATA data = _makeOrderData(msg);
    data.setTimer = setTimer;
    data.orderID = _orderID;

    _allOrders[_orderID] = data;
    _gid2OrderIDs[data.groupID].push_back(_orderID);

    if (data.action == TRADE_ACTION_BUYOPEN ||
        data.action == TRADE_ACTION_SELLOPEN)
    {
        _openOrderIDList.push_back(_orderID);
    }

    if (data.action == TRADE_ACTION_BUYCLOSE ||
        data.action == TRADE_ACTION_SELLCLOSE)
    {
        _closeOrderIDList.push_back(_orderID);
    }
    _showData();

}
void TradeStrategy::_clearTrade(int orderID)
{
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[clearTrade]";
    info << "|orderID|" << orderID;
    info << endl;
    info.close();

    int gid = 0;
    std::vector<int>::iterator i;
    std::map<int, std::vector<int> >::iterator i2V;
    std::map<int, ORDER_DATA>::iterator i2O = _allOrders.find(orderID);
    if (i2O != _allOrders.end()) {
        gid = (i2O->second).groupID;
        _allOrders.erase(i2O);
    }
    if (gid > 0) {
        i2V = _gid2OrderIDs.find(gid);
        std::vector<int> tmp;
        if (i2V != _gid2OrderIDs.end()) {
            tmp = i2V->second;
            for (i = tmp.begin(); i != tmp.end(); i++) {
                if ((*i) == orderID) {
                    tmp.erase(i);
                    break;
                }
            }
        }
        if (tmp.size() == 0) {
            _gid2OrderIDs.erase(i2V);
        }
    }
    for (i = _openOrderIDList.begin(); i != _openOrderIDList.end(); i++) {
        if ((*i) == orderID) {
            _openOrderIDList.erase(i);
            break;
        }
    }
    for (i = _closeOrderIDList.begin(); i != _closeOrderIDList.end(); i++) {
        if ((*i) == orderID) {
            _closeOrderIDList.erase(i);
            break;
        }
    }
    for (i = _cancelOrderIDList.begin(); i != _cancelOrderIDList.end(); i++) {
        if ((*i) == orderID) {
            _cancelOrderIDList.erase(i);
            break;
        }
    }
    _showData();
}

void TradeStrategy::_sendMsg(double price, int total, bool isBuy, bool isOpen, int orderID)
{
    ORDER_DATA order = _allOrders[orderID];

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
    info << "|groupID|" << order.groupID;
    info << "|forecastType|" << order.forecastType;
    info << endl;
    info.close();

    MSG_TO_TRADE msg = {0};
    msg.msgType = MSG_ORDER;
    msg.price = price;
    msg.isBuy = isBuy;
    msg.total = total;
    msg.isOpen = isOpen;
    msg.orderID = orderID;
    msg.forecastType = order.forecastType;
    strcpy(msg.instrumnetID, order.instrumnetID);
    _tradeSrvClient->send((void *)&msg);

}

ORDER_DATA TradeStrategy::_makeOrderData(MSG_TO_TRADE_STRATEGY msg)
{
    ORDER_DATA data = {0};
    data.groupID = msg.groupID;
    data.kIndex = msg.kIndex;
    data.price = msg.price;
    data.total = msg.total;
    data.forecastType = msg.forecastType;
    strcpy(data.instrumnetID, msg.instrumnetID);
    switch (msg.msgType) {
        case MSG_TRADE_BUYOPEN:
            data.action = TRADE_ACTION_BUYOPEN;
            break;
        case MSG_TRADE_SELLOPEN:
            data.action = TRADE_ACTION_SELLOPEN;
            break;
        case MSG_TRADE_BUYCLOSE:
            data.action = TRADE_ACTION_BUYCLOSE;
            break;
        case MSG_TRADE_SELLCLOSE:
            data.action = TRADE_ACTION_SELLCLOSE;
            break;
        default:
            break;
    }
    data.msg = msg;
    data.setTimer = false;
    return data;
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

void TradeStrategy::_showData()
{
    std::vector<int>::iterator i;
    std::map<int, std::vector<int> >::iterator i2V;
    std::map<int, ORDER_DATA>::iterator i2O;

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[data]";
    info << "|allOrders|";
    for (i2O = _allOrders.begin(); i2O != _allOrders.end(); ++i2O)
    {
        info << i2O->first << ",";
    }
    info << "|gid2OrderIDs|";
    for (i2V = _gid2OrderIDs.begin(); i2V != _gid2OrderIDs.end(); ++i2V)
    {
        info << i2V->first << "->";
        for (i = (i2V->second).begin(); i != (i2V->second).end(); ++i)
        {
            info << *i << "-";
        }
        info << ",";
    }
    info << "|cancelOrderIDList|";
    for (i = _cancelOrderIDList.begin(); i != _cancelOrderIDList.end(); ++i)
    {
        info << *i << ",";
    }
    info << "|closeOrderIDList|";
    for (i = _closeOrderIDList.begin(); i != _closeOrderIDList.end(); ++i)
    {
        info << *i << ",";
    }
    info << "|openOrderIDList|";
    for (i = _openOrderIDList.begin(); i != _openOrderIDList.end(); ++i)
    {
        info << *i << ",";
    }

    info << endl;
    info.close();
}
