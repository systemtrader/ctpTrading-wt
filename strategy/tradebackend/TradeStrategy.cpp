#include "TradeStrategy.h"

timer_t timer;
extern int timeoutSec;
extern TradeStrategy * ts; 

void timeout(union sigval v)
{
    ts->timeout(v.sival_int);
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


TradeStrategy::TradeStrategy()
{
    _cancelFlag = false;
    _store = new Redis("127.0.0.1", 6379, 1);
}

TradeStrategy::~TradeStrategy()
{
    delete _store;
}


void TradeStrategy::tradeAction(int action, double price, int total)
{
    int status = _getStatus();
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
    Lib::initInfoLogHandle(info);
    info << "TradeStrategy[successBack]" << "|";
    info << "status" << "|" << _getStatus() endl;
    info.close();
}

void TradeStrategy::_cancelBack()
{
    if (_needZ()) {
        _zhuijia();
    }
}

void TradeStrategy::timeout()
{
    _cancelAction();
}

void TradeStrategy::_cancelAction()
{
    // 
}

void TradeStrategy::_needZ()
{
    return !_cancelFlag;
}

int TradeStrategy::_getStatus()
{
    string status = _store->get("TRADE_STATUS");
    return Lib::stoi(status);
}

void TradeStrategy::_zhuijia()
{
    double price;
    Tick tick = _getTick();
    int status = _getStatus();
    switch (status) {
        case TRADE_STATUS_SELLOPENING:
        case TRADE_STATUS_BUYOPENING:
            price = tick.price;
            // TODO 发送消息
            break;
        case TRADE_STATUS_SELLCLOSING:
            price = tick.price - 30;
            break;
        case TRADE_STATUS_BUYCLOSING:
            price = tick.price + 30;
            break;
        default:
            break; 
    }
}