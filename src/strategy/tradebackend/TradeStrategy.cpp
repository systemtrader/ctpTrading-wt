#include "TradeStrategy.h"

timer_t timer;
extern int timeoutSec;
extern TradeStrategy * service;

void timeout(union sigval v)
{
    service->timeout();
    return;
}

void setTimer(int action)
{
    // 设定定时器
    struct sigevent evp;
    struct itimerspec ts;

    memset(&evp, 0, sizeof(evp));
    evp.sigev_notify = SIGEV_THREAD;
    evp.sigev_value.sival_int = action;
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


void TradeStrategy::tradeAction(int action, double price, int total)
{
    int status = _getStatus();
    _currentOrderID++;
    switch (action) {

        case TRADE_ACTION_BUYOPEN:
            _setStatus(TRADE_STATUS_BUYOPENING);
            switch (status) {
                case TRADE_STATUS_SELLOPENING:
                    _cancelAction();
                case TRADE_STATUS_NOTHING:
                    _sendMsg();
                    break;
                case TRADE_STATUS_BUYOPENING:
                default:
                    break;
            }
            break;

        case TRADE_ACTION_SELLOPEN:
            _setStatus(TRADE_STATUS_SELLOPENING);
            switch (status) {
                case TRADE_STATUS_BUYOPENING:
                    _cancelAction();
                case TRADE_STATUS_NOTHING:
                    _sendMsg();
                    break;
                case TRADE_STATUS_SELLOPENING:
                default:
                    break;
            }
            break;

        case TRADE_ACTION_BUYCLOSE:
            _setStatus(TRADE_STATUS_BUYCLOSING);
            _sendMsg();
            break;

        case TRADE_ACTION_SELLCLOSE:
            _setStatus(TRADE_STATUS_SELLCLOSING);
            _sendMsg();
            break;
        default:
            _cancelAction();
            break;
    }
    // 启动定时器
    setTimer(action);
}

void TradeStrategy::onTradeMsgBack(bool isSuccess)
{
    if (isSuccess) {
        _successBack();
    } else {
        _cancelBack();
    }
}

void TradeStrategy::_successBack()
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

    // log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeStrategySrv[successBack]" << "|";
    info << "status" << "|" << _getStatus() << endl;
    info.close();
}

void TradeStrategy::_cancelBack()
{
    if (_orderingID == 0 || _currentOrderID == _orderingID) {
        _zhuijia();
    }
}

void TradeStrategy::timeout()
{
    _cancelAction();
    if (_orderingID == 0 || _currentOrderID == _orderingID) {
        _zhuijia();
    }
}

void TradeStrategy::_cancelAction()
{
    _sendMsg();
}

void TradeStrategy::_zhuijia()
{
    _orderingID = _currentOrderID;
    double price;
    TickData tick = _getTick();
    int status = _getStatus();
    switch (status) {
        case TRADE_STATUS_SELLOPENING:
        case TRADE_STATUS_BUYOPENING:
            price = tick.price;
            // TODO 发送消息
            _sendMsg();
            break;
        case TRADE_STATUS_SELLCLOSING:
            price = tick.price - 30;
            _sendMsg();
            break;
        case TRADE_STATUS_BUYCLOSING:
            price = tick.price + 30;
            _sendMsg();
            break;
        default:
            break;
    }
}

void TradeStrategy::_sendMsg()
{

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
