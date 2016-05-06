#ifndef TRADE_STRATEGY_H
#define TRADE_STRATEGY_H

#include "../../global.h"
#include "../global.h"
#include "../../libs/Redis.h"
#include <sys/time.h>
#include <signal.h>
#include <cstring>
#include <map>
#include <list>

typedef struct waiting_data
{
    int action;
    double price;
    int total;
    int orderID;

} WAITING_DATA;

class TradeStrategy
{
private:

    Redis * _store;
    QClient * _tradeSrvClient;
    string _logPath;

    map<int, int> _tradingOrders;
    list<int> _waitingOrders;
    map<int, WAITING_DATA> _waitingOrdersInfo;

    void _cancelBack(int);
    void _successBack(int);

    void _zhuijia(int); // 追价
    void _cancel(int); // 撤销

    void _clearTradingOrder(int);

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
