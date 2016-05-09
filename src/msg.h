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

} MSG_TO_KLINE;


// kLineSrv->tradeLogicSrv
#define MSG_KLINE_OPEN  3
#define MSG_KLINE_CLOSE 4
typedef struct msg_k_line
{
    long int msgType;
    KLineBlockData block;
    TickData tick;

} MSG_TO_TRADE_LOGIC;

// tradeLogicSrv->tradeStrategySrv
#define MSG_TRADE_BUYOPEN   5
#define MSG_TRADE_SELLOPEN  6
#define MSG_TRADE_BUYCLOSE  7
#define MSG_TRADE_SELLCLOSE 8
#define MSG_TRADE_CANCEL    9
// tradeSrv->tradeStrategySrv
#define MSG_TRADE_BACK_TRADED   10
#define MSG_TRADE_BACK_CANCELED 11
typedef struct msg_trade_data
{
    long int msgType;
    double price;
    int total;
    int kIndex;
    int hasNext;
    char instrumnetID[7];

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

} MSG_TO_TRADE;

#endif
