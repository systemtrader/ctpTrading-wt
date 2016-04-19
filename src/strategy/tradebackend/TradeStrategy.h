#ifndef TRADE_STRATEGY_H
#define TRADE_STRATEGY_H

#include "../global.h"
#include "../Tick.h"
#include "../../libs/Redis.h"
#include "../../libs/Lib.h"
#include <string>
#include <iostream>
#include <sys/time.h>
#include <signal.h>
#include <cstring>

using namespace std;

class TradeStrategy
{
private:

    Redis * _store;

    int _orderingID;
    int _currentOrderID;

    void _cancelBack();
    void _successBack();
    void _cancelAction();

    // 追价
    void _zhuijia();

    Tick _getTick();

    int _getStatus();
    void _setStatus(int status);

    void _sendMsg();

public:
    TradeStrategy();
    ~TradeStrategy();

    void tradeAction(int action, double price, int total = 1);
    void onTradeMsgBack(bool isSuccess);
    void timeout();

};



#endif
