#ifndef TRADE_SRV_H
#define TRADE_SRV_H

#include "../../include/ThostFtdcTraderApi.h"
#include "../strategy/global.h"
#include "../global.h"
#include "../libs/Redis.h"
#include <map>


class TraderSpi;

class TradeSrv
{
private:

    string _logPath;
    Redis * _store;

    string _brokerID;
    string _userID;
    string _password;
    string _instrumnetID;
    string _tradeFront;
    string _flowPath;

    string _tradingDay;
    TThostFtdcFrontIDType _frontID;
    TThostFtdcSessionIDType _sessionID;
    int _maxOrderRef;

    bool _ydPostion; // 昨仓
    int _closeYdReqID; // 平昨仓的reqID

    CThostFtdcTraderApi * _tradeApi;
    TraderSpi * _traderSpi;
    QClient * _tradeStrategySrvClient;

    void _initOrderRefAndID(int);
    int _getOrderRefByID(int);
    int _getOrderIDByRef(int);

    map<int, CThostFtdcOrderField> _orderInfoMap; 
    void _setOrderInfo(int, CThostFtdcOrderField * const);
    CThostFtdcOrderField _getOrderInfo(int);

    CThostFtdcInputOrderField _createOrder(int, bool, int, double,
        TThostFtdcOffsetFlagEnType, // 开平标志
        TThostFtdcHedgeFlagEnType = THOST_FTDC_HFEN_Speculation, // 投机套保标志
        TThostFtdcOrderPriceTypeType = THOST_FTDC_OPT_LimitPrice, // 报单价格条件
        TThostFtdcTimeConditionType = THOST_FTDC_TC_IOC, // 有效期类型
        TThostFtdcVolumeConditionType = THOST_FTDC_VC_CV, //成交量类型
        TThostFtdcContingentConditionType = THOST_FTDC_CC_Immediately// 触发条件
    );

public:

    TradeSrv(string, string, string, string, string, string, string, int);
    ~TradeSrv();

    void setStatus(int);

    void init();

    void login();
    void onLogin(CThostFtdcRspUserLoginField * const);

    void getPosition();
    void onPositionRtn(CThostFtdcInvestorPositionField * const);

    void trade(double, int, bool, bool, int);
    void onTraded(CThostFtdcTradeField * const);
    void onOrderRtn(CThostFtdcOrderField * const);

    void cancel(int);
    void onCancel(CThostFtdcInputOrderActionField * const);

};

#endif
