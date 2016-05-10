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

typedef struct trade_data
{
    int action;
    int hasNext;
    int tryTimes;
    int kIndex;
    string instrumnetID;

} TRADE_DATA;

class TradeStrategy
{
private:

    Redis * _store;
    QClient * _tradeSrvClient;
    string _logPath;
    int _orderID;

    std::map<int, TRADE_DATA> _tradingInfo;
    void _removeTradeInfo(int);
    bool _isTrading(int);

    void _initTrade(int, int, int, string); // 初始化交易
    void _zhuijia(int); // 追价
    void _cancel(int); // 撤销

    int _getStatus(string);
    void _setStatus(int);
    TickData _getTick();
    void _sendMsg(double, int, bool, bool, int);

public:
    TradeStrategy(int, string, int);
    ~TradeStrategy();

    void tradeAction(int, double, int, int, int, string);
    void onSuccess(int);
    void onCancel(int);
    void timeout(int);

};



#endif
