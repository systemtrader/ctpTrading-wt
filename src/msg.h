#ifndef MSG_H
#define MSG_H

#include "protos/Data.h"

#define MSG_SHUTDOWN 99

// market->klineSrv
#define MSG_TICK 2
typedef struct msg_tick
{
    long int msgType;
    TickData tick;
    bool isMy;

} MSG_TO_KLINE;


// kLineSrv->tradeLogicSrv
#define MSG_KLINE_OPEN  3
#define MSG_KLINE_CLOSE 4
#define MSG_KLINE_CLOSE_BY_ME 21
#define MSG_LOGIC_ROLLBACK 20
#define MSG_LOGIC_REALBACK 22
#define MSG_TRADE_END 23
#define MSG_TRADE_TICK 24
#define MSG_TRADE_FORECAST_SUCCESS 25
#define MSG_TRADE_ONE_FORECAST_SUCCESS 26
typedef struct msg_k_line
{
    long int msgType;
    KLineBlockData block;
    TickData tick;
    int forecastID;
    int kIndex;
} MSG_TO_TRADE_LOGIC;

// tradeLogicSrv->tradeStrategySrv
#define MSG_TRADE_BUYOPEN   5
#define MSG_TRADE_SELLOPEN  6
#define MSG_TRADE_BUYCLOSE  7
#define MSG_TRADE_SELLCLOSE 8
#define MSG_TRADE_ROLLBACK  15

#define MSG_TRADE_FORECAST_OVER 14
#define MSG_TRADE_REAL_COME 16

#define MSG_TRADE_BACK_TRADED   10
#define MSG_TRADE_BACK_CANCELED 11
#define MSG_TRADE_BACK_ERR 17

#define TRADE_TYPE_NORMAL 0
#define TRADE_TYPE_FOK 1
#define TRADE_TYPE_FAK 2
#define TRADE_TYPE_IOC 3

typedef struct msg_trade_data
{
    long int msgType;
    double price;
    int total;
    int kIndex;
    bool isForecast; // 是否是预测单
    int forecastID; // 预测id
    int statusWay;
    // bool isFok;
    int type;
    // int groupID; // 逻辑模块生成
    int orderID; // 下单模块生成，下单系统反馈时使用
    // int forecastType;
    char instrumnetID[7];
    int err;
    TThostFtdcDateType date;
    TThostFtdcTimeType time;


} MSG_TO_TRADE_STRATEGY;

// tradeStrategySrv->tradeSrv
#define MSG_ORDER 12
#define MSG_ORDER_CANCEL 13
typedef struct msg_trade
{
    long int msgType;
    double price;
    bool isBuy;
    int total;
    bool isOpen;
    int orderID;
    int forecastType;
    char instrumnetID[7];
    // bool isFok;
    int type;

} MSG_TO_TRADE;

#endif
