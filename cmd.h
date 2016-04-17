#ifndef CMD_H
#define CMD_H

#include <string>
using namespace std;

const string CMD_MSG_SHUTDOWN = "0";
const string CMD_MSG_START = "1";

// const string CMD_MSG_TRADE_FOK = "2";
const string CMD_MSG_TICK = "4";

const string CMD_MSG_KLINE_OPEN = "5";
const string CMD_MSG_KLINE_CLOSE = "6";

const string CMD_MSG_TRADE_BUYOPEN = "7";
const string CMD_MSG_TRADE_SELLOPEN = "8";
const string CMD_MSG_TRADE_BUYCLOSE = "9";
const string CMD_MSG_TRADE_SELLCLOSE = "10";
const string CMD_MSG_TRADE_CANCEL = "13";

const string CMD_MSG_ON_TRADED = "11";
const string CMD_MSG_ON_CANCELED = "12";

#endif
