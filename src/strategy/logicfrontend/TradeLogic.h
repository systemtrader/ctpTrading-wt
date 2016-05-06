#ifndef TRADE_LOGIC_H
#define TRADE_LOGIC_H

#include "../../global.h"
#include "../global.h"
#include "../../libs/Redis.h"
#include "../../protos/KLineBlock.h"
#include <list>

#define TRANS_TYPE_UP2UP     0
#define TRANS_TYPE_UP2DOWN   1
#define TRANS_TYPE_DOWN2UP   2
#define TRANS_TYPE_DOWN2DOWN 3

class TradeLogic
{
private:

    int _isHistoryBack;
    Redis * _store;
    QClient * _tradeStrategySrvClient;
    string _logPath;

    // 算法参数
    int _peroid; // 周期
    double _threshold; // 阈值

    // 计算辅助变量
    int _kIndex;
    int _countUp2Up;
    int _countUp2Down;
    int _countDown2Up;
    int _countDown2Down;
    list<int> _transTypeList; // 保存transType
    list<TickData> _tickGroup; // 保存三个tick数据，满足三个tick则可以计算出一个transType，即可存入transTypeList

    double _pUp2Up;
    double _pUp2Down;
    void _calculateUp();

    double _pDown2Up;
    double _pDown2Down;
    void _calculateDown();

    void _tick(TickData); // 处理tick信息

    // 判断仓位状态
    int _getStatus();
    void _sendMsg(int, double = 0);

public:
    TradeLogic(int, double, int, string, int, int);
    ~TradeLogic();

    void init(); // 初始化历史K线

    void onKLineClose(KLineBlock, TickData);
};

#endif
