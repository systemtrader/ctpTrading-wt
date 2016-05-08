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

void TradeStrategy::_initTrade(int action, int kIndex, int hasNext)
{
    _orderID++;
    TRADE_DATA order = {0};
    order.action = action;
    order.hasNext = hasNext;
    order.tryTimes = 5;
    order.kIndex = kIndex;
    _tradingInfo[_orderID] = order;

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[initTrade]";
    info << "|kIndex|" << kIndex;
    info << "|orderID|" << _orderID;
    info << endl;
    info.close();

    // save data
    string data = "klineorder_" + Lib::itos(kIndex) + "_" + Lib::itos(_orderID);
    _store->push("ORDER_LOGS", data);
}

void TradeStrategy::_removeTradeInfo(int orderID)
{
    std::map<int, TRADE_DATA>::iterator i;
    i = _tradingInfo.find(orderID);
    if (i != _tradingInfo.end()) {
        TRADE_DATA order = i->second;
        if (!order.hasNext) {
            switch (order.action) {
                case TRADE_ACTION_BUYOPEN:
                case TRADE_ACTION_SELLOPEN:
                    _setStatus(TRADE_STATUS_NOTHING);
                    break;
                case TRADE_ACTION_BUYCLOSE:
                    _setStatus(TRADE_STATUS_BUYOPENED);
                case TRADE_ACTION_SELLCLOSE:
                    _setStatus(TRADE_STATUS_SELLOPENED);
                    break;
                default:
                    break;
            }
        }
        _tradingInfo.erase(i);
        // log
        int status = _getStatus();
        ofstream info;
        Lib::initInfoLogHandle(_logPath, info);
        info << "TradeStrategySrv[removeTrade]";
        info << "|kIndex|" << order.kIndex;
        info << "|orderID|" << orderID;
        info << "|status|" << status;
        info << endl;
        info.close();
    }
}

bool TradeStrategy::_isTrading(int orderID)
{
    std::map<int, TRADE_DATA>::iterator i;
    i = _tradingInfo.find(orderID);
    return i == _tradingInfo.end() ? false : true;
}

void TradeStrategy::tradeAction(int action, double price, int total, int kIndex, int hasNext)
{
    _initTrade(action, kIndex, hasNext);
    switch (action) {

        case TRADE_ACTION_BUYOPEN:
            _setStatus(TRADE_STATUS_BUYOPENING);
            _sendMsg(price, total, true, true, _orderID);
            break;

        case TRADE_ACTION_SELLOPEN:
            _setStatus(TRADE_STATUS_SELLOPENING);
            _sendMsg(price, total, false, true, _orderID);
            break;

        case TRADE_ACTION_BUYCLOSE:
            _setStatus(TRADE_STATUS_BUYCLOSING);
            _sendMsg(price, total, true, false, _orderID);
            break;

        case TRADE_ACTION_SELLCLOSE:
            _setStatus(TRADE_STATUS_SELLCLOSING);
            _sendMsg(price, total, false, false, _orderID);
            break;

        default:
            break;
    }
    // 启动定时器
    setTimer(_orderID);
}

void TradeStrategy::onSuccess(int orderID)
{
    TRADE_DATA order = _tradingInfo[orderID];

    _removeTradeInfo(orderID);
    if (!order.hasNext) {
        switch (order.action) {
            case TRADE_ACTION_BUYOPEN:
                _setStatus(TRADE_STATUS_BUYOPENED);
                break;
            case TRADE_ACTION_SELLOPEN:
                _setStatus(TRADE_STATUS_SELLOPENED);
                break;
            case TRADE_ACTION_BUYCLOSE:
            case TRADE_ACTION_SELLCLOSE:
                _setStatus(TRADE_STATUS_NOTHING);
                break;
            default:
                break;
        }
    }

    // log
    int status = _getStatus();
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[successBack]";
    info << "|kIndex|" << order.kIndex;
    info << "|orderID|" << orderID;
    info << "|status|" << status;
    info << endl;
    info.close();
}

void TradeStrategy::onCancel(int orderID)
{
    TRADE_DATA order = _tradingInfo[orderID];

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[cancelBack]";
    info << "|kIndex|" << order.kIndex;
    info << "|orderID|" << orderID;
    info << endl;
    info.close();

    if (_isTrading(orderID)) {
        _zhuijia(orderID);
    }
}

void TradeStrategy::timeout(int orderID)
{
    if (_isTrading(orderID)) {
        TRADE_DATA order = _tradingInfo[orderID];

        ofstream info;
        Lib::initInfoLogHandle(_logPath, info);
        info << "TradeStrategySrv[timeout]";
        info << "|kIndex|" << order.kIndex;
        info << "|orderID|" << orderID;
        info << endl;
        info.close();
        _cancel(orderID);
    }

}

void TradeStrategy::_cancel(int orderID)
{
    TRADE_DATA order = _tradingInfo[orderID];

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[cancel]";
    info << "|kIndex|" << order.kIndex;
    info << "|orderID|" << orderID;
    info << endl;
    info.close();

    MSG_TO_TRADE msg = {0};
    msg.msgType = MSG_ORDER_CANCEL;
    msg.orderID = orderID;
    _tradeSrvClient->send((void *)&msg);

    order.tryTimes--;
    if (order.tryTimes == 0) {
        _removeTradeInfo(orderID);
    } else {
        _tradingInfo[orderID] = order;
    }

}

void TradeStrategy::_zhuijia(int orderID)
{
    TRADE_DATA order = _tradingInfo[orderID];

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[zhuijia]";
    info << "|kIndex|" << order.kIndex;
    info << "|orderID|" << orderID;
    info << endl;
    info.close();

    double price;
    TickData tick = _getTick();
    switch (order.action) {
        case TRADE_ACTION_SELLOPEN:
            price = tick.price;
            _sendMsg(price, 1, false, true, orderID);
            break;
        case TRADE_ACTION_BUYOPEN:
            price = tick.price;
            _sendMsg(price, 1, true, true, orderID);
            break;
        case TRADE_ACTION_SELLCLOSE:
            price = tick.price - 10;
            _sendMsg(price, 1, false, false, orderID);
            break;
        case TRADE_ACTION_BUYCLOSE:
            price = tick.price + 10;
            _sendMsg(price, 1, true, false, orderID);
            break;
        default:
            break;
    }
    // 启动定时器
    setTimer(orderID);

    order.tryTimes--;
    if (order.tryTimes == 0) {
        _removeTradeInfo(orderID);
    } else {
        _tradingInfo[orderID] = order;
    }
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

    TRADE_DATA order = _tradingInfo[orderID];
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[sendOrder]";
    info << "|price|" << price;
    info << "|total|" << total;
    info << "|isBuy|" << isBuy;
    info << "|isOpen|" << isOpen;
    info << "|kIndex|" << order.kIndex;
    info << "|orderID|" << orderID;
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
