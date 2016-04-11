#ifndef TRADE_LOGIC_H
#define TRADE_LOGIC_H

#include "../global.h"
#include <string>
#include <iostream>

using namespace std;

#define CLOSE_ACTION_OPEN 0
#define CLOSE_ACTION_SELLCLOSE 1
#define CLOSE_ACTION_BUYCLOSE 2


class TradeLogic
{
private:

    int closeAction;

    int getStatus();
    int

public:
    TradeLogic();
    ~TradeLogic();

    void onKLineOpen();
    void onKLineClose();
};

#endif
