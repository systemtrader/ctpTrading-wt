#ifndef TRADE_STRATEGY_H
#define TRADE_STRATEGY_H

#include "../global.h"
#include <string>
#include <iostream>

using namespace std;

class TradeStrategy
{
private:

    Redis * _store;

    void _cancelBack();
    void _successBack();
    void _timeout();

    // 是否需要追价
    int _needZ();

    int _getStatus();
    void _setStatus(int status);

public:
    TradeStrategy();
    ~TradeStrategy();

    void tradeAction(int action);
    void onTradeMsgBack(int status);

};



#endif
