#ifndef TRADE_LOGIC_H
#define TRADE_LOGIC_H

#include "../../global.h"
#include "../global.h"
#include "../../libs/Redis.h"
#include "../../protos/KLineBlock.h"
#include <list>

class TradeLogic
{
private:

    int _isHistoryBack;
    Redis * _store;
    QClient * _tradeStrategySrvClient;

    int _closeAction;
    list<KLineBlock> _bList;

    // 算法参数
    int _openMaxKLineCount; // 开仓用K线条数
    int _openMinKLineCount; // 开仓用K线条数
    int _openMeanKLineCount; // 开仓用K线条数
    int _kRang; // K线赋值
    int _closeSellKRangeCount; // 卖平仓K线条数
    int _closeBuyKRangeCount; // 买平仓K线条数

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

    string _logPath;

    // 判断仓位状态
    int _getStatus();
    // 计算开仓参数
    void _calculateOpen();
    // 计算卖平仓参数
    void _calculateSellClose();
    // 计算买平仓参数
    void _calculateBuyClose();

    void _sendMsg(int, double = 0);

    TickData _getTick();

public:
    TradeLogic(int, int, int, int, int, int, int, string, int);
    ~TradeLogic();

    void init(); // 初始化历史K线

    void onKLineOpen();
    void onKLineClose(KLineBlock block);
};

#endif
