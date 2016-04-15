#include "TradeStrategy.h"

TradeStrategy::TradeStrategy()
{
    _store = new Redis("127.0.0.1", 6379, 1);
}

TradeStrategy::~TradeStrategy()
{
    delete _store;
}


void TradeStrategy::tradeAction(int action, double price, int total = 1)
{
    switch (action) {

        case TRADE_ACTION_BUYOPEN:
            _setStatus(TRADE_STATUS_BUYOPENING);
            // TODO send msg
            break;

        case TRADE_ACTION_SELLOPEN:
            _setStatus(TRADE_STATUS_SELLOPENING);
            break;

        case TRADE_ACTION_BUYCLOSE:
            _setStatus(TRADE_STATUS_BUYCLOSING);
            break;

        case TRADE_ACTION_SELLCLOSE:
            _setStatus(TRADE_STATUS_SELLCLOSING);
            break;
        default:
            break;
    }
    // log
    ofstream info;
    Lib::initInfoLogHandle(info);
    info << "tradeAction" << "|";
    info << "ACTION" << "|" << action << "|";
    info << "PRICE" << "|" << price << endl;
    info.close();
}
