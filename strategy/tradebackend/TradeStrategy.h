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
    bool _cancelFlag;

    void _cancelBack();
    void _successBack();
    void _cancelAction();

    // 是否需要追价
    int _needZ();
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
    void timeout(int action);

};



#endif
