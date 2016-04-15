#ifndef TRADE_LOGIC_H
#define TRADE_LOGIC_H

#include "../global.h"
#include "../KLineBlock.h"
#include "../../libs/Lib.h"
#include "../../libs/Redis.h"
#include <string>
#include <list>
#include <iostream>

using namespace std;

#define CLOSE_ACTION_OPEN 0
#define CLOSE_ACTION_SELLCLOSE 1
#define CLOSE_ACTION_BUYCLOSE 2
#define CLOSE_ACTION_DONOTHING 3

class TradeLogic
{
private:

    int _closeAction;
    Redis * _store;
    list<KLineBlock> _bList;

    // 算法参数
    int _kLineCountMax;
    int _kLineCountMin;
    int _kLineCountMean;
    int _openIndex; // 开仓K线

    // 开仓参数
    double _max;
    double _mean;
    double _min;


    // 判断仓位状态
    int _getStatus();
    // 计算开仓参数
    void _calculateOpen();
    // 计算卖平仓参数
    void _calculateSellClose();
    // 计算买平仓参数
    void _calculateBuyClose();

    // 计算最大
    void _getSpecialKLine(int * maxPos, int * minPos, double * maxPrice, double * minPrice);


public:
    TradeLogic(int countMax, int countMin, int countMean);
    ~TradeLogic();

    void init(); // 初始化历史K线

    void onKLineOpen();
    void onKLineClose(KLineBlock block);
};

#endif
