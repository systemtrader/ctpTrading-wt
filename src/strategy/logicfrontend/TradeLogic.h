#ifndef TRADE_LOGIC_H
#define TRADE_LOGIC_H

#include "../global.h"
#include "../KLineBlock.h"
#include "../../libs/Lib.h"
#include "../../libs/Redis.h"
#include "../Tick.h"
#include <string>
#include <list>
#include <iostream>

using namespace std;

class TradeLogic
{
private:

    const char * _cfdIp;
    int _cfdPort;

    int _closeAction;
    Redis * _store;
    list<KLineBlock> _bList;

    // 算法参数
    int _openMaxKLineNum; // 开仓用K线条数
    int _openMinKLineNum; // 开仓用K线条数
    int _openMeanKLineNum; // 开仓用K线条数
    int _kRang; // K线赋值
    int _sellCloseKLineNum; // 卖平仓K线条数
    int _buyCloseKLineNum; // 买平仓K线条数

    // 开仓判断依据
    double _max;
    double _mean;
    double _min;

    // 平仓判断依据
    double _sellClosePoint;
    double _buyClosePoint;

    // 开仓情况下，当前K线组峰、谷值
    double _openedKLineMax;
    double _openedKLineMin;


    // 判断仓位状态
    int _getStatus();
    // 计算开仓参数
    void _calculateOpen();
    // 计算卖平仓参数
    void _calculateSellClose();
    // 计算买平仓参数
    void _calculateBuyClose();

    void _sendMsg(string msg, double price = 0);

    Tick _getTick();

public:
    TradeLogic(int countMax, int countMin, int countMean, int kRang,
        int sellCloseKLineNum, int buyCloseKLineNum);
    ~TradeLogic();

    void init(); // 初始化历史K线

    void onKLineOpen();
    void onKLineClose(KLineBlock block);
};

#endif
