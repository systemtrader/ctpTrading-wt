#ifndef TRADE_LOGIC_H
#define TRADE_LOGIC_H

#include "../../global.h"
#include "../global.h"
#include "../../libs/Redis.h"
#include "../../protos/KLineBlock.h"
#include <list>
#include <map>

#define TRANS_TYPE_UP2UP     0
#define TRANS_TYPE_UP2DOWN   1
#define TRANS_TYPE_DOWN2UP   2
#define TRANS_TYPE_DOWN2DOWN 3


#define STATUS_TYPE_CLOSE_NO 0
#define STATUS_TYPE_CLOSE_BOED 1
#define STATUS_TYPE_CLOSE_SOED 2
#define STATUS_TYPE_CLOSE_BOING 3
#define STATUS_TYPE_CLOSE_SOING 4
#define STATUS_TYPE_CLOSE_BOING_SOING 5

#define STATUS_TYPE_CLOSE_SCING 6
#define STATUS_TYPE_CLOSE_BCING 7
#define STATUS_TYPE_CLOSE_SOING_SCING 8
#define STATUS_TYPE_CLOSE_SOING_NO 9
#define STATUS_TYPE_CLOSE_BOING_BCING 10
#define STATUS_TYPE_CLOSE_BOING_NO 11

#define STATUS_TYPE_MYCLOSE_BOED 12
#define STATUS_TYPE_MYCLOSE_SOED 13
#define STATUS_TYPE_MYCLOSE_BOED_SOING 14
#define STATUS_TYPE_MYCLOSE_BOING_SOED 15

#define STATUS_TYPE_MYCLOSE_NO 16
#define STATUS_TYPE_MYCLOSE_SOED_SCING 17
#define STATUS_TYPE_MYCLOSE_SOED_NO 18
#define STATUS_TYPE_MYCLOSE_BOED_BCING 19
#define STATUS_TYPE_MYCLOSE_BOED_NO 20


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

    int _forecastID;
    int _rollbackOpenUUID;
    int _rollbackOpenUDID;
    int _rollbackOpenDDID;
    int _rollbackOpenDUID;
    int _rollbackCloseUID;
    int _rollbackCloseDID;
    void _setRollbackID(int, int);
    int _kRange; // 放弃

    // 算法参数
    int _peroid; // 周期
    double _thresholdTrend; // 阈值
    double _thresholdVibrate;

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

    void _tick(TickData); // 处理tick信息

    bool _isCurrentUp();

    int _statusType;
    void _rollback();

    TickData _closeTick;
    void _forecast(TickData);
    void _forecastNothing(TickData);
    void _forecastSellOpened(TickData);
    void _forecastBuyOpened(TickData);
    void _realAction(TickData);
    void _onKLineCloseDo(TickData);
    void _sendRollBack(int);

    // 判断仓位状态
    TickData _getTick();
    int _getStatus(int);
    void _setStatus(int, int);
    void _sendMsg(int, double, bool, int, int, bool = false);

public:
    TradeLogic(int, double, double, int, string, int, string, string, int);
    ~TradeLogic();

    void init(); // 初始化历史K线

    void onKLineClose(KLineBlock, TickData);
    // void onKLineOpen(KLineBlock, TickData);
    void onKLineCloseByMe(KLineBlock, TickData);
    void onRollback();
    void onRealActionBack();
};

#endif

