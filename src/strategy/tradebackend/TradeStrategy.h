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

    bool _isSelfCancel;

    int _currentOrderID;
    int _doingOrderID;

    void _cancelBack(int);
    void _successBack(int);
    void _cancelAction(int);

    // 追价
    void _zhuijia();

    TickData _getTick();

    int _getStatus();
    void _setStatus(int);

    void _sendMsg(double, int, bool, bool);

public:
    TradeStrategy(int, string);
    ~TradeStrategy();

    void tradeAction(int, double, int, int);
    void onTradeMsgBack(bool, int);
    void timeout(int);

};



#endif
