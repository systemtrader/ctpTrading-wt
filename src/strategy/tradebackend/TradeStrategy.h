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

typedef struct order_data
{
    int orderID;
    int groupID;
    int kIndex;
    int action;
    double price;
    int total;
    int forecastType;
    char instrumnetID[7];
    MSG_TO_TRADE_STRATEGY msg;
    bool setTimer;
} ORDER_DATA;

class TradeStrategy
{
private:

    Redis * _store;
    QClient * _tradeSrvClient;
    QClient * _klineClient;
    string _logPath;
    int _orderID;

    ORDER_DATA _makeOrderData(MSG_TO_TRADE_STRATEGY);

    std::vector<MSG_TO_TRADE_STRATEGY> _waitingList; // 等待预测结束的暂存
    std::map<int, ORDER_DATA> _allOrders; // 全部订单信息
    std::map<int, std::vector<int> > _gid2OrderIDs; // 操作组
    std::vector<int> _openOrderIDList;
    std::vector<int> _closeOrderIDList;
    std::vector<int> _cancelOrderIDList;

    map<int, int> _orderIDDealed; // orderID -> 1
    map<int, int> _orderIDCanceled; // orderID -> 1

    void _findOrder2Send(bool = false); // 从List中寻找可以发送的订单并发送
    void _initTrade(MSG_TO_TRADE_STRATEGY, bool = false); // 初始化订单
    void _clearTrade(int); // 清空订单
    void _clearOpen();

    void _open(ORDER_DATA);
    void _close(ORDER_DATA);
    void _cancel(int);
    void _zhuijia(ORDER_DATA);

    void _dealForecast();
    void _dealRealCome();
    void _rollback(int);

    void _showData();
    int _getStatus(string);
    void _setStatus(int, string);
    TickData _getTick(string);
    void _sendMsg(double, int, bool, bool, int);

public:
    TradeStrategy(int, string, int, int);
    ~TradeStrategy();

    void accessAction(MSG_TO_TRADE_STRATEGY);
    void onSuccess(int);
    void onCancel(int);
    void onCancelErr(int);
    void timeout(int);

};



#endif
