#ifndef TRADE_STRATEGY_H
#define TRADE_STRATEGY_H

#include "../../global.h"
#include "../global.h"
#include "../../libs/Redis.h"
#include <sys/time.h>
#include <signal.h>
#include <cstring>
#include <list>

class TradeStrategy
{
private:

    Redis * _store;
    QClient * _tradeSrvClient;
    string _logPath;

    list<int> _tradingOrderID;

    void _cancelBack(int);
    void _successBack(int);

    void _clearTrade(); // 清空未完成的交易
    void _zhuijia(int); // 追价
    void _cancelAction(int); // 撤销

    bool _isInOrderIDList(list<int>, int);
    list<int> _removeList(list<int>, int);

    int _getStatus();
    void _setStatus(int);
    TickData _getTick();
    void _sendMsg(double, int, bool, bool, int);

public:
    TradeStrategy(int, string, int);
    ~TradeStrategy();

    void tradeAction(int, double, int, int);
    void onTradeMsgBack(bool, int);
    void timeout(int);

};



#endif
