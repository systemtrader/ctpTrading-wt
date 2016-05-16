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
    if (msg.msgType == MSG_TRADE_ROLLBACK) {
        _rollback(msg.groupID);
        return;
    }
    if (msg.msgType == MSG_TRADE_FORECAST_OVER) {
        _dealForecast();
        return;
    }
    if (msg.msgType == MSG_TRADE_REAL_COME) {
        _dealRealCome();
        return;
    }
    _waitingList.push_back(msg);
}

void TradeStrategy::_dealForecast()
{
    int closeCnt = 0;
    // 处理开仓、撤单，立即执行
    std::vector<MSG_TO_TRADE_STRATEGY>::iterator it;
    for (it = _waitingList.begin(); it != _waitingList.end(); it++) {
        switch (it->msgType) {
            case MSG_TRADE_BUYOPEN:
            case MSG_TRADE_SELLOPEN:
                _open(*it);
                _waitingList.erase(it);
                break;
            case MSG_TRADE_SELLCLOSE:
            case MSG_TRADE_BUYCLOSE:
                closeCnt++;
            default:
                break;
        }
    }

    // 处理平仓，由于可能对同一笔开仓进行两笔平仓操作，
    // 所以要看同一次操作中有几个平仓，若两个以上，则放弃，等待tick满足条件再平仓
    if (closeCnt = 1) {
        _close(*(_waitingList.begin()));
    }
    _waitingList.clear();

}

void TradeStrategy::_dealRealCome()
{
    std::vector<MSG_TO_TRADE_STRATEGY>::iterator it;
    for (it = _waitingList.begin(); it != _waitingList.end(); it++) {
        _close(*it);
        setTimer(_orderID);
    }
    _waitingList.clear();
}

int TradeStrategy::_initTrade(MSG_TO_TRADE_STRATEGY data)
{
    _orderID++; // 生成订单ID
    _group2orderMap[data.groupID] = _orderID;
    _orderDetail[_orderID] = data;

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[initTrade]";
    info << "|iID|" << data.instrumnetID;
    info << "|kIndex|" << data.kIndex;
    info << "|orderID|" << _orderID;
    info << endl;
    info.close();
    return _orderID;
}

void TradeStrategy::_open(MSG_TO_TRADE_STRATEGY data)
{
    int orderID = _initTrade(data);

    if (data.msgType == MSG_TRADE_BUYOPEN) {
        _sendMsg(data.price, data.total, true, true, orderID);
    }
    if (data.msgType == MSG_TRADE_SELLOPEN) {
        _sendMsg(data.price, data.total, false, true, orderID);
    }
}

void TradeStrategy::_close(MSG_TO_TRADE_STRATEGY data)
{
    int orderID = _initTrade(data);

    if (data.msgType == MSG_TRADE_BUYCLOSE) {
        _sendMsg(data.price, data.total, true, false, orderID);
    }
    if (data.msgType == MSG_TRADE_SELLCLOSE) {
        _sendMsg(data.price, data.total, false, false, orderID);
    }
}

void TradeStrategy::_rollback(int groupID)
{
    int orderID = _group2orderMap[groupID];
    if (!orderID) return;
    _cancel(orderID);
}

void TradeStrategy::_cancel(int orderID)
{
    MSG_TO_TRADE_STRATEGY order = _orderDetail[orderID];

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[cancel]";
    info << "|iID|" << order.instrumnetID;
    info << "|kIndex|" << order.kIndex;
    info << "|orderID|" << orderID;
    info << "|groupID|" << order.groupID;
    info << endl;
    info.close();

    MSG_TO_TRADE msg = {0};
    msg.msgType = MSG_ORDER_CANCEL;
    msg.orderID = orderID;
    _tradeSrvClient->send((void *)&msg);
}

void TradeStrategy::_zhuijia(int orderID)
{
    MSG_TO_TRADE_STRATEGY order = _orderDetail[orderID];

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[zhuijia]";
    info << "|iID|" << order.instrumnetID;
    info << "|kIndex|" << order.kIndex;
    info << "|orderID|" << orderID;
    info << "|groupID|" << order.groupID;
    info << endl;
    info.close();

    int newOrderID = _initTrade(order);

    double price;
    TickData tick = _getTick(order.instrumnetID);
    switch (order.msgType) {
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
    // 启动定时器
    setTimer(newOrderID);
}


void TradeStrategy::_removeOrderInfo(int orderID)
{
    bool isOK = false;
    MSG_TO_TRADE_STRATEGY data = {0};
    std::map<int, MSG_TO_TRADE_STRATEGY>::iterator i;
    i = _orderDetail.find(orderID);
    if (i != _orderDetail.end()) {
        data = i->second;
        _orderDetail.erase(i);

        std::map<int, int>::iterator it;
        it = _group2orderMap.find(data.groupID);
        if (it != _group2orderMap.end()) {
            _group2orderMap.erase(it);
            isOK = true;
        }
    }
    
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[removeTrade]";
    info << "|iID|" << data.instrumnetID;
    info << "|kIndex|" << data.kIndex;
    info << "|orderID|" << orderID;
    info << "|groupID|" << data.groupID;
    info << "|isSuccess|" << isOK;
    info << endl;
    info.close();
}

bool TradeStrategy::_isTrading(int orderID)
{
    std::map<int, MSG_TO_TRADE_STRATEGY>::iterator i;
    i = _orderDetail.find(orderID);
    return i == _orderDetail.end() ? false : true;
}

void TradeStrategy::onSuccess(int orderID)
{
    MSG_TO_TRADE_STRATEGY order = _orderDetail[orderID];

    _removeOrderInfo(orderID);
    switch (order.msgType) {
        case MSG_TRADE_BUYOPEN:
            _setStatus(TRADE_STATUS_BUYOPENED, order.instrumnetID);
            break;
        case MSG_TRADE_SELLOPEN:
            _setStatus(TRADE_STATUS_SELLOPENED, order.instrumnetID);
            break;
        case MSG_TRADE_BUYCLOSE:
        case MSG_TRADE_SELLCLOSE:
            _setStatus(TRADE_STATUS_NOTHING, order.instrumnetID);
            break;
        default:
            break;
    }

    // 生成一个Tick，发送给K线系统
    MSG_TO_KLINE msg = {0};
    msg.msgType = MSG_TICK;
    msg.tick.price = order.price;
    msg.tick.bidPrice1 = order.price;
    msg.tick.askPrice1 = order.price;
    strcpy(msg.tick.instrumnetID, order.instrumnetID);
    _klineClient->send((void *)&msg);

    // 将数据放入队列，以便存入DB
    string tickStr = Lib::tickData2String(msg.tick);
    string keyQ = "MARKET_TICK_Q";
    string keyD = "CURRENT_TICK_" + string(order.instrumnetID);
    _store->set(keyD, tickStr); // tick数据，供全局使用
    _store->push(keyQ, tickStr);

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
    MSG_TO_TRADE_STRATEGY order = _orderDetail[orderID];

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[cancelBack]";
    info << "|iID|" << order.instrumnetID;
    info << "|kIndex|" << order.kIndex;
    info << "|orderID|" << orderID;
    info << "|groupID|" << order.groupID;
    info << endl;
    info.close();

    _removeOrderInfo(orderID);
}

void TradeStrategy::timeout(int orderID)
{
    if (_isTrading(orderID)) {
    
        MSG_TO_TRADE_STRATEGY order = _orderDetail[orderID];

        ofstream info;
        Lib::initInfoLogHandle(_logPath, info);
        info << "TradeStrategySrv[timeout]";
        info << "|iID|" << order.instrumnetID;
        info << "|kIndex|" << order.kIndex;
        info << "|orderID|" << orderID;
        info << endl;
        info.close();

        _zhuijia(orderID);
        _cancel(orderID);
    }

}

void TradeStrategy::_sendMsg(double price, int total, bool isBuy, bool isOpen, int orderID)
{
    MSG_TO_TRADE_STRATEGY order = _orderDetail[orderID];

    MSG_TO_TRADE msg = {0};
    msg.msgType = MSG_ORDER;
    msg.price = price;
    msg.isBuy = isBuy;
    msg.total = total;
    msg.isOpen = isOpen;
    msg.orderID = orderID;
    msg.forecastType = order.forecastType;
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
    info << "|forecastType|" << order.forecastType;
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
