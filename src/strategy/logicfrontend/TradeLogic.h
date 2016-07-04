#ifndef TRADE_LOGIC_H
#define TRADE_LOGIC_H

#include "../../global.h"
#include "../global.h"
#include "../../libs/Redis.h"
#include "../../protos/KLineBlock.h"
#include <list>
#include <map>
#include <cmath>

#define TRANS_TYPE_UP2UP     0
#define TRANS_TYPE_UP2DOWN   1
#define TRANS_TYPE_DOWN2UP   2
#define TRANS_TYPE_DOWN2DOWN 3


#define STATUS_TYPE_CLOSE_NO 0
#define STATUS_TYPE_CLOSE_BOED 1
#define STATUS_TYPE_CLOSE_SOED 2
#define STATUS_TYPE_CLOSE_BOING_SOING 3
#define STATUS_TYPE_CLOSE_NO_SOING 4
#define STATUS_TYPE_CLOSE_BOING_NO 5
#define STATUS_TYPE_CLOSE_BOING 21
#define STATUS_TYPE_CLOSE_SOING 22

#define STATUS_TYPE_CLOSE_WAIT_BOED 30
#define STATUS_TYPE_CLOSE_WAIT_SOED 31
#define STATUS_TYPE_CLOSE_WAIT_BOED_SOING 32
#define STATUS_TYPE_CLOSE_WAIT_BOING_SOED 33


#define STATUS_TYPE_CLOSE_SCING 6
#define STATUS_TYPE_CLOSE_BCING 7
#define STATUS_TYPE_CLOSE_SOING__SCING 8
#define STATUS_TYPE_CLOSE_SOING__NO 9
#define STATUS_TYPE_CLOSE_BOING__BCING 10
#define STATUS_TYPE_CLOSE_BOING__NO 11

#define STATUS_TYPE_CLOSE_WAIT_NO 34
#define STATUS_TYPE_CLOSE_WAIT_SOED__SCING 35
#define STATUS_TYPE_CLOSE_WAIT_SOED__NO 36
#define STATUS_TYPE_CLOSE_WAIT_BOED__BCING 37
#define STATUS_TYPE_CLOSE_WAIT_BOED__NO 38



#define STATUS_TYPE_MYCLOSE_BOED_SOING 12
#define STATUS_TYPE_MYCLOSE_BOING_SOED 13
#define STATUS_TYPE_MYCLOSE_NO_SOED 14
#define STATUS_TYPE_MYCLOSE_BOED_NO 15
#define STATUS_TYPE_MYCLOSE_BOED 23
#define STATUS_TYPE_MYCLOSE_SOED 24

#define STATUS_TYPE_MYCLOSE_NO 16
#define STATUS_TYPE_MYCLOSE_SOED__SCING 17
#define STATUS_TYPE_MYCLOSE_SOED__NO 18
#define STATUS_TYPE_MYCLOSE_BOED__BCING 19
#define STATUS_TYPE_MYCLOSE_BOED__NO 20


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
    std::vector<TRADE_HM> _stopHM; // 停止交易时间
    std::vector<TRADE_HM> _startHM; // 停止交易时间

    bool _isLock;
    double _lineRatio;
    int _serial;
    int _actionKIndex;

    double _minRange;

    int _forecastID;
    int _rollbackOpenUUID;
    int _rollbackOpenUDID;
    int _rollbackOpenDDID;
    int _rollbackOpenDUID;
    int _rollbackCloseUID;
    int _rollbackCloseDID;
    void _setRollbackID(int, int);
    int _kRange; // 放弃
    bool _isTradeEnd;

    double _forecastOpenPrice1;
    double _forecastOpenPrice2;
    double _forecastClosePrice;

    // 算法参数
    int _peroid; // 周期
    double _thresholdTrend; // 阈值
    double _thresholdVibrate;

    // 计算辅助变量
    int _kIndex;
    // int _countUp2Up;
    // int _countUp2Down;
    // int _countDown2Up;
    // int _countDown2Down;
    list<int> _transTypeList; // 保存transType
    list<TickData> _tickGroup; // 保存三个tick数据，满足三个tick则可以计算出一个transType，即可存入transTypeList

    double _pUp2Up;
    double _pUp2Down;
    void _calculateUp(int = -1);

    double _pDown2Up;
    double _pDown2Down;
    void _calculateDown(int = -1);

    void _tick(TickData); // 处理tick信息

    bool _isCurrentUp();

    int _statusType;
    bool _rollback();

    TickData _closeTick;
    void _forecast(TickData);
    void _forecastNothing(TickData);
    void _forecastSellOpened(TickData);
    void _forecastBuyOpened(TickData);
    void _realAction(TickData);
    void _onKLineCloseDo(TickData);
    void _sendRollBack(int);

    void _endClose();
    bool _isSerial();

    bool _canRollbackClose(TickData, bool);
    bool _canRollbackOpen(TickData);
    void _setTickSwitch(bool);

    bool _isTradingTime(TickData);

    // 判断仓位状态
    TickData _getTick();
    int _getStatus(int);
    void _setStatus(int, int);
    void _sendMsg(int, double, bool, int, int, bool = false);

public:
    TradeLogic(int, double, double, int, string, int, string, string, string, int, int, int);
    ~TradeLogic();

    void init(); // 初始化历史K线

    void onKLineClose(KLineBlock, TickData);
    // void onKLineOpen(KLineBlock, TickData);
    void onKLineCloseByMe(KLineBlock, TickData);
    void onRollback();
    void onRealActionBack();
    void onTradeEnd();
    void onTick(TickData);
    void onForecastSuccess();
};

#endif

