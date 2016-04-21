#ifndef TRADE_STRATEGY_H
#define TRADE_STRATEGY_H

#include "../../global.h"
#include "../global.h"
#include "../../libs/Redis.h"
#include <sys/time.h>
#include <signal.h>
#include <cstring>

class TradeStrategy
{
private:

    Redis * _store;
    QClient * _tradeSrvClient;
    string _logPath;

    int _orderingID;
    int _currentOrderID;

    void _cancelBack();
    void _successBack();
    void _cancelAction();

    // 追价
    void _zhuijia();

    TickData _getTick();

    int _getStatus();
    void _setStatus(int);

    void _sendMsg(double, int, bool, bool, bool = false);

public:
    TradeStrategy(int, string);
    ~TradeStrategy();

    void tradeAction(int, double, int = 1);
    void onTradeMsgBack(bool);
    void timeout();

};



#endif
