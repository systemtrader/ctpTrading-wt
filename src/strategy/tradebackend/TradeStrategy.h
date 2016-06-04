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
    double price;
    int total;
    int kIndex;
    int forecastID;
    bool isForecast;
    bool isMain;
    string instrumnetID;

} TRADE_DATA;

class TradeStrategy
{
private:

    Redis * _store;
    QClient * _tradeSrvClient;
    QClient * _klineClient;
    string _logPath;

    int _orderID;
    std::map<int, TRADE_DATA> _tradingInfo; // orderID -> tradeInfo
    bool _isTrading(int);
    std::map<int, int> _forecastID2OrderID;
    bool _isForecasting(int);

    std::list<MSG_TO_TRADE_STRATEGY> _waitList;

    int _initTrade(int, int, int, string, double, int, bool, bool); // 初始化交易
    void _clearTradeInfo(int);

    void _tradeAction(MSG_TO_TRADE_STRATEGY);
    void _zhuijia(int); // 追价
    void _cancel(int); // 撤销

    int _getStatus(string);
    void _setStatus(int, string);
    TickData _getTick(string);
    void _sendMsg(double, int, bool, bool, int);

public:
    TradeStrategy(int, string, int, int);
    ~TradeStrategy();

    void trade(MSG_TO_TRADE_STRATEGY);
    void onSuccess(int, double);
    void onCancel(int);
    void onCancelErr(int);
    void timeout(int);

};



#endif
