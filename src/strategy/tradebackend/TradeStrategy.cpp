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

TradeStrategy::TradeStrategy(int serviceID, string logPath)
{
    _logPath = logPath;
    _orderingID = _currentOrderID = 0;
    _store = new Redis("127.0.0.1", 6379, 1);
    _tradeSrvClient = new QClient(serviceID, sizeof(MSG_TO_TRADE));

}

TradeStrategy::~TradeStrategy()
{
    delete _store;
    delete _tradeSrvClient;
    cout << "~TradeStrategy" << endl;
}


void TradeStrategy::tradeAction(int action, double price, int total, int kIndex)
{
    int status = _getStatus();
    _currentOrderID = kIndex;
    switch (action) {

        case TRADE_ACTION_BUYOPEN:
            _setStatus(TRADE_STATUS_BUYOPENING);
            switch (status) {
                case TRADE_STATUS_SELLOPENING:
                case TRADE_STATUS_BUYOPENING:
                    _cancelAction(_doingOrderID);
                case TRADE_STATUS_NOTHING:
                    _setStatus(TRADE_STATUS_BUYOPENING);
                    _sendMsg(price, total, true, true);
                    break;
                default:
                    break;
            }
            break;

        case TRADE_ACTION_SELLOPEN:
            _setStatus(TRADE_STATUS_SELLOPENING);
            switch (status) {
                case TRADE_STATUS_BUYOPENING:
                case TRADE_STATUS_SELLOPENING:
                    _cancelAction(_doingOrderID);
                case TRADE_STATUS_NOTHING:
                    _setStatus(TRADE_STATUS_SELLOPENING);
                    _sendMsg(price, total, false, true);
                    break;
                default:
                    break;
            }
            break;

        case TRADE_ACTION_BUYCLOSE:
            _setStatus(TRADE_STATUS_BUYCLOSING);
            _sendMsg(price, total, true, false);
            break;

        case TRADE_ACTION_SELLCLOSE:
            _setStatus(TRADE_STATUS_SELLCLOSING);
            _sendMsg(price, total, false, false);
            break;
        default:
            _cancelAction();
            break;
    }
    // 启动定时器
    setTimer(_currentOrderID);
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

    // todo 记录交易

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[successBack]" << "|";
    info << "kIndex" << "|" << orderID << "|";
    info << "status" << "|" << _getStatus() << endl;
    info.close();
}

void TradeStrategy::_cancelBack(int orderID)
{
    if (_orderingID == 0 || _currentOrderID == _orderingID) {
        _zhuijia();
    }
}

void TradeStrategy::timeout(int orderID)
{
    _cancelAction(orderID);
    if (orderID == _currentOrderID) {
        _zhuijia();
    }
}

void TradeStrategy::_cancelAction()
{
    _sendMsg(0, 0, true, true, true);
}

void TradeStrategy::_zhuijia()
{
    double price;
    TickData tick = _getTick();
    int status = _getStatus();
    switch (status) {
        case TRADE_STATUS_SELLOPENING:
            price = tick.price;
            _sendMsg(price, 1, false, true);
            break;
        case TRADE_STATUS_BUYOPENING:
            price = tick.price;
            _sendMsg(price, 1, true, true);
            break;
        case TRADE_STATUS_SELLCLOSING:
            price = tick.price - 30;
            _sendMsg(price, 1, false, false);
            break;
        case TRADE_STATUS_BUYCLOSING:
            price = tick.price + 30;
            _sendMsg(price, 1, true, false);
            break;
        default:
            break;
    }
}

void TradeStrategy::_sendMsg(double price, int total, bool isBuy, bool isOpen, bool isCancel)
{
    MSG_TO_TRADE msg = {0};
    msg.msgType = MSG_ORDER;
    msg.price = price;
    msg.isBuy = isBuy;
    msg.total = total;
    msg.isOpen = isOpen;
    msg.isCancel = isCancel;
    clt.send((void *)&msg);

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[sendOrder]" << "|";
    info << "price" << "|" << price << "|";
    info << "total" << "|" << total << "|";
    info << "isBuy" << "|" << isBuy << "|";
    info << "isOpen" << "|" << isOpen << endl;
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
