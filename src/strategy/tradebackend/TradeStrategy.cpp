#include "TradeStrategy.h"

timer_t timer;
extern int timeoutSec;
extern std::map<string, TradeStrategy* > services;
std::map<int, string> orderID2iID;

void timeout(union sigval v)
{
    string iID = orderID2iID[v.sival_int];
    services[iID]->timeout(v.sival_int);
    return;
}

void setTimer(int orderID, string iID)
{
    orderID2iID[orderID] = iID;
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

TradeStrategy::TradeStrategy(int serviceID, string logPath, int db, int serviceIDK, int serviceIDL, int minRange, int kRange)
{
    _orderID = 0;
    _logPath = logPath;
    _store = new Redis("127.0.0.1", 6379, db);
    _storeTick = new Redis("127.0.0.1", 6379, 1);
    _tradeSrvClient = new QClient(serviceID, sizeof(MSG_TO_TRADE));
    _klineClient = new QClient(serviceIDK, sizeof(MSG_TO_KLINE));
    _tradeLogicSrvClient = new QClient(serviceIDL, sizeof(MSG_TO_TRADE_LOGIC));
    _minRange = (double)minRange;
    _kRange = kRange;
    _rollbackCnt = 0;
}

TradeStrategy::~TradeStrategy()
{
    // delete _store;
    // delete _tradeSrvClient;
    cout << "~TradeStrategy" << endl;
}

int TradeStrategy::_initTrade(int action, int kIndex, int total, string instrumnetID,
    double price, int forecastID, bool isForecast, int statusWay, bool isZhuijia)
{
    _orderID = Lib::stoi(_store->incr("ORDER_ID_MAX"));

    TRADE_DATA order = {0};
    order.action = action;
    order.price = price;
    order.kIndex = kIndex;
    order.total = total;
    order.instrumnetID = instrumnetID;
    order.forecastID = forecastID;
    order.isForecast = isForecast;
    order.statusWay = statusWay;
    _tradingInfo[_orderID] = order;

    _forecastID2OrderID[forecastID] = _orderID;

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, instrumnetID);
    info << "TradeStrategySrv[initTrade]";
    info << "|orderID|" << _orderID;
    info << "|forecastID|" << forecastID;
    info << "|isForecast|" << isForecast;
    info << "|statusWay|" << statusWay;
    info << "|kIndex|" << kIndex;
    info << endl;
    info.close();

    // save data
    string data = "klineorder_" + Lib::itos(kIndex) + "_" + Lib::itos(_orderID) + "_" + instrumnetID + "_" + Lib::itos(isForecast)
                 + "_" + Lib::itos(isZhuijia) + "_" + Lib::itos(_kRange);
    _store->push("ORDER_LOGS", data);

    return _orderID;
}

bool TradeStrategy::_isRollback(int orderID)
{
    std::map<int, int>::iterator i = _rollbackID.find(orderID);
    return i == _rollbackID.end() ? false : true;
}

void TradeStrategy::_clearRollbackID(int orderID)
{
    std::map<int, int>::iterator j = _rollbackID.find(orderID);
    if (j != _rollbackID.end()) _rollbackID.erase(j);
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
    int status1 = _getStatus(1, string(msg.instrumnetID));
    int status2 = _getStatus(2, string(msg.instrumnetID));
    int status3 = _getStatus(3, string(msg.instrumnetID));
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, string(msg.instrumnetID));
    info << "TradeStrategySrv[tradeCome]";
    info << "|status1|" << status1;
    info << "|status2|" << status2;
    info << "|status3|" << status3;
    info << "|msgType|" << msg.msgType;
    info << "|forecastID|" << msg.forecastID;
    info << endl;
    info.close();

    // 撤单直接发送
    if (msg.msgType == MSG_TRADE_ROLLBACK) {
        if (msg.forecastID == -1) { // 回滚停止标志
            if (_rollbackCnt == 0) { // 没有发生回滚，应对发出回滚信号时订单已经全部成交
                MSG_TO_TRADE_LOGIC rsp = {0};
                rsp.msgType = MSG_LOGIC_ROLLBACK;
                strcpy(rsp.tick.instrumnetID, msg.instrumnetID);
                _tradeLogicSrvClient->send((void *)&rsp);
            }
            _rollbackCnt = 0;
            return;
        }
        if (!_isForecasting(msg.forecastID)) return;
        _rollbackCnt++;
        int orderID = _forecastID2OrderID[msg.forecastID];
        _rollbackID[orderID] = 1; // 记录rollback的orderID
        _cancel(orderID, 2);
        return;
    }

    // 预测单，直接发送
    if (msg.isForecast) {
        _tradeAction(msg);
        return;
    }

    // 通道1的请求可能出现排队
    if (status1 == TRADE_STATUS_BUYOPENING ||
        status1 == TRADE_STATUS_BUYCLOSING ||
        status1 == TRADE_STATUS_SELLOPENING ||
        status1 == TRADE_STATUS_SELLCLOSING)
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

    int orderID = _initTrade(action, kIndex, total, instrumnetID, price, forecastID, msg.isForecast, msg.statusWay);
    switch (action) {

        case TRADE_ACTION_BUYOPEN:
            _setStatus(msg.statusWay, TRADE_STATUS_BUYOPENING, instrumnetID);
            _sendMsg(price, total, true, true, orderID, msg.isFok);
            break;

        case TRADE_ACTION_SELLOPEN:
            _setStatus(msg.statusWay, TRADE_STATUS_SELLOPENING, instrumnetID);
            _sendMsg(price, total, false, true, orderID, msg.isFok);
            break;

        case TRADE_ACTION_BUYCLOSE:
            _setStatus(msg.statusWay, TRADE_STATUS_BUYCLOSING, instrumnetID);
            _sendMsg(price, total, true, false, orderID);
            break;

        case TRADE_ACTION_SELLCLOSE:
            _setStatus(msg.statusWay, TRADE_STATUS_SELLCLOSING, instrumnetID);
            _sendMsg(price, total, false, false, orderID);
            break;

        default:
            break;
    }
    if (!msg.isForecast && (action == TRADE_ACTION_BUYCLOSE || action == TRADE_ACTION_SELLCLOSE))
        setTimer(orderID, instrumnetID);
}

void TradeStrategy::onSuccess(MSG_TO_TRADE_STRATEGY rsp)
{
    int orderID = rsp.orderID;
    if (!_isTrading(orderID)) return;
    TRADE_DATA order = _tradingInfo[orderID];

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, order.instrumnetID);
    info << "TradeStrategySrv[successBack]";
    info << "|orderID|" << orderID;
    info << "|statusWay|" << order.statusWay;
    info << "|isForecast|" << order.isForecast;
    info << "|waitingSize|" << _waitList.size();
    info << endl;
    info.close();

    _clearTradeInfo(orderID);

    // 主线单更改主状态
    if (_waitList.size() == 0) {

        switch (order.action) {
            case TRADE_ACTION_BUYOPEN:
                _setStatus(order.statusWay, TRADE_STATUS_BUYOPENED, order.instrumnetID);
                break;
            case TRADE_ACTION_SELLOPEN:
                _setStatus(order.statusWay, TRADE_STATUS_SELLOPENED, order.instrumnetID);
                break;
            case TRADE_ACTION_BUYCLOSE:
            case TRADE_ACTION_SELLCLOSE:
                _setStatus(order.statusWay, TRADE_STATUS_NOTHING, order.instrumnetID);
                break;
            default:
                break;
        }

        if (_isRollback(orderID)) { // 回滚的订单成交了
            _clearRollbackID(orderID);
            if (_rollbackID.size() == 0) { // 全部都回滚了
                MSG_TO_TRADE_LOGIC msg = {0};
                msg.msgType = MSG_LOGIC_ROLLBACK;
                strcpy(msg.tick.instrumnetID, order.instrumnetID.c_str());
                _tradeLogicSrvClient->send((void *)&msg);
            }
        } else if (order.isForecast) {
            if (order.statusWay == 1 || order.statusWay == 2) {

                Lib::initInfoLogHandle(_logPath, info, order.instrumnetID);
                info << "TradeStrategySrv[sendMyTick]";
                info << "|orderID|" << orderID;
                info << "|statusWay|" << order.statusWay;
                info << "|price|" << rsp.price;
                info << "|date|" << rsp.date;
                info << "|time|" << rsp.time;
                info << endl;
                info.close();

                // 生成一个Tick，发送给K线系统
                MSG_TO_KLINE msg = {0};
                msg.msgType = MSG_TICK;
                msg.tick.price = rsp.price;
                msg.tick.volume = 0;
                msg.tick.bidPrice1 = rsp.price;
                msg.tick.bidVolume1 = 0;
                msg.tick.askPrice1 = rsp.price;
                msg.tick.askVolume1 = 0;
                strcpy(msg.tick.date, rsp.date);
                strcpy(msg.tick.time, rsp.time);
                msg.tick.msec = -1;
                strcpy(msg.tick.instrumnetID, order.instrumnetID.c_str());
                msg.isMy = true;
                _klineClient->send((void *)&msg);

                // 将数据放入队列，以便存入DB
                string tickStr = Lib::tickData2String(msg.tick);
                string keyQ = "MARKET_TICK_Q";
                string keyD = "CURRENT_TICK_" + string(order.instrumnetID);
                _store->set(keyD, tickStr); // tick数据，供全局使用
                _storeTick->push(keyQ, tickStr);
            }

        } else {
            MSG_TO_TRADE_LOGIC msg = {0};
            msg.msgType = MSG_LOGIC_REALBACK;
            strcpy(msg.tick.instrumnetID, order.instrumnetID.c_str());
            _tradeLogicSrvClient->send((void *)&msg);
        }

    } else {

        MSG_TO_TRADE_STRATEGY msg = _waitList.front();
        _waitList.pop_front();
        _tradeAction(msg);

    }
}

void TradeStrategy::onCancel(MSG_TO_TRADE_STRATEGY rsp)
{
    int orderID = rsp.orderID;
    if (!_isTrading(orderID)) return;
    TRADE_DATA order = _tradingInfo[orderID];

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, order.instrumnetID);
    info << "TradeStrategySrv[cancelBack]";
    info << "|orderID|" << orderID;
    info << "|statusWay|" << order.statusWay;
    info << "|isForecast|" << order.isForecast;
    info << "|waitingSize|" << _waitList.size();
    info << endl;
    info.close();

    // 非预测单的平仓需要追价
    if ((order.action == TRADE_ACTION_SELLCLOSE ||
         order.action == TRADE_ACTION_BUYCLOSE) &&
        !order.isForecast)
    {
        _zhuijia(orderID, rsp.price);
        _clearTradeInfo(orderID);
        return;
    }

    // 主线单有机会改状态
    if (_waitList.size() == 0) {
        switch (order.action) {
            case TRADE_ACTION_BUYOPEN:
            case TRADE_ACTION_SELLOPEN:
                _setStatus(order.statusWay, TRADE_STATUS_NOTHING, order.instrumnetID);
                break;
            case TRADE_ACTION_BUYCLOSE:
                _setStatus(order.statusWay, TRADE_STATUS_SELLOPENED, order.instrumnetID);
                break;
            case TRADE_ACTION_SELLCLOSE:
                _setStatus(order.statusWay, TRADE_STATUS_BUYOPENED, order.instrumnetID);
                break;
            default:
                break;
        }

        if (_isRollback(orderID)) { // 回滚的订单成交了
            _clearRollbackID(orderID);
            if (_rollbackID.size() == 0) { // 全部都回滚了
                MSG_TO_TRADE_LOGIC msg = {0};
                msg.msgType = MSG_LOGIC_ROLLBACK;
                strcpy(msg.tick.instrumnetID, order.instrumnetID.c_str());
                _tradeLogicSrvClient->send((void *)&msg);
            }
        }
        if (!order.isForecast) { // 实时单，对于开仓，撤销以后给反馈
            if (order.action == TRADE_ACTION_BUYOPEN ||
                order.action == TRADE_ACTION_SELLOPEN)
            {
                MSG_TO_TRADE_LOGIC msg = {0};
                msg.msgType = MSG_LOGIC_REALBACK;
                strcpy(msg.tick.instrumnetID, order.instrumnetID.c_str());
                _tradeLogicSrvClient->send((void *)&msg);
            }
        }


    } else {

        MSG_TO_TRADE_STRATEGY msg = _waitList.front();
        _waitList.pop_front();
        _tradeAction(msg);
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
        _setStatus(order.statusWay, TRADE_STATUS_NOTHING, order.instrumnetID);
        if (_waitList.size() > 0) {
            MSG_TO_TRADE_STRATEGY msg = _waitList.front();
            _waitList.pop_front();
            _tradeAction(msg);
        }
    }
}

void TradeStrategy::timeout(int orderID)
{
    if (!_isTrading(orderID)) return;
    TRADE_DATA order = _tradingInfo[orderID];

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, order.instrumnetID);
    info << "TradeStrategySrv[timeout]";
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
    Lib::initInfoLogHandle(_logPath, info, order.instrumnetID);
    info << "TradeStrategySrv[cancel]";
    info << "|orderID|" << orderID;
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

void TradeStrategy::_zhuijia(int orderID, double price)
{
    if (!_isTrading(orderID)) return;
    TRADE_DATA order = _tradingInfo[orderID];

    int newOrderID = _initTrade(order.action, order.kIndex, order.total, order.instrumnetID, order.price, order.forecastID, order.isForecast, order.statusWay, true);

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, order.instrumnetID);
    info << "TradeStrategySrv[zhuijia]";
    info << "|orderID|" << orderID;
    info << "|newOrderID|" << newOrderID;
    info << "|kIndex|" << order.kIndex;
    info << endl;
    info.close();

    TickData tick = _getTick(order.instrumnetID);
    switch (order.action) {
        case TRADE_ACTION_SELLCLOSE:
            price = tick.bidPrice1 < price - _minRange ? tick.bidPrice1 : price - _minRange;
            _sendMsg(price, 1, false, false, newOrderID);
            break;
        case TRADE_ACTION_BUYCLOSE:
            price = tick.askPrice1 > price + _minRange ? tick.askPrice1 : price + _minRange;
            _sendMsg(price, 1, true, false, newOrderID);
            break;
        default:
            break;
    }
    setTimer(newOrderID, order.instrumnetID);
}

void TradeStrategy::_sendMsg(double price, int total, bool isBuy, bool isOpen, int orderID, bool isFok)
{
    TRADE_DATA order = _tradingInfo[orderID];

    MSG_TO_TRADE msg = {0};
    msg.msgType = MSG_ORDER;
    msg.price = price;
    msg.isBuy = isBuy;
    msg.total = total;
    msg.isOpen = isOpen;
    msg.isFok = isFok;
    msg.orderID = orderID;
    strcpy(msg.instrumnetID, Lib::stoc(order.instrumnetID));
    _tradeSrvClient->send((void *)&msg);

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, order.instrumnetID);
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

TickData TradeStrategy::_getTick(string iID)
{
    string tickStr = _store->get("CURRENT_TICK_" + iID);
    return Lib::string2TickData(tickStr);
}

int TradeStrategy::_getStatus(int way, string instrumnetID)
{
    string status = _store->get("TRADE_STATUS_" + Lib::itos(way) + "_" + instrumnetID);
    return Lib::stoi(status);
}

void TradeStrategy::_setStatus(int way, int status, string instrumnetID)
{
    _store->set("TRADE_STATUS_" + Lib::itos(way) + "_" + instrumnetID, Lib::itos(status));
}
