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

typedef struct trade_hm
{
    int hour;
    int min;
} TRADE_HM;

class TradeLogic
{
private:

    Redis * _store;
    QClient * _tradeStrategySrvClient;
    string _logPath;

    string _instrumnetID; // 合约代码
    std::vector<TRADE_HM> _timeHM; // 停止交易时间

    // 算法参数
    int _peroid; // 周期
    double _threshold; // 阈值
    int _kRange;

    // 订单组id
    int _groupID;
    int _forecastUp;
    int _forecastDown;

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
    void _calculateUp(double, double);

    double _pDown2Up;
    double _pDown2Down;
    void _calculateDown(double, double);

    bool _isCurrentUp(TickData);

    // 预测开仓、平仓
    void _forecastNothing(TickData);
    void _forecastSellOpened(TickData);
    void _forecastBuyOpened(TickData);

    // 撤单操作
    void _rollbackAll();
    void _rollbackUp();
    void _rollbackDown();

    void _tick(TickData); // 处理tick信息

    // 判断仓位状态
    int _getStatus();
    void _sendMsg(int, double = 0);
    void _sendRollBack(int);
    void _sendRealCome(int);

public:
    TradeLogic(int, double, int, string, int, string, string, int);
    ~TradeLogic();

    void init(); // 初始化历史K线

    void onKLineClose(KLineBlock, TickData);
    void onKLineOpen(KLineBlock, TickData);
};

#endif
