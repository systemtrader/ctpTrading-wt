#include "global.h"
#include "../ThostFtdcTraderApi.h"
#include "../lib.h"
#include <string>
#include <iostream>

using namespace std;

class TradeAction
{

public:

    TradeAction(CThostFtdcTraderApi * tApi) 
    {
        _tApi = tApi;
    };

    ~TradeAction() 
    {
        _tApi = NULL;
        cout << "~TradeAction" << endl;
    };
    
    void tradeFOK(string exchangeID, string instrumnetID, 
        int isBuy, int total, double price, int offsetType)
    {
        TThostFtdcOffsetFlagEnType offsetFlag;
        switch (offsetType) {
            case 1:
                offsetFlag = THOST_FTDC_OFEN_Open;
                break;
            case 2:
                offsetFlag = THOST_FTDC_OFEN_Close;
                break;
            case 3:
                offsetFlag = THOST_FTDC_OFEN_CloseToday;
                break;
            case 4:
                offsetFlag = THOST_FTDC_OFEN_ForceClose;
                break;
            default:
                break;
        }
        CThostFtdcInputOrderField order = createOrder(exchangeID, instrumnetID, 
            isBuy, total, price, offsetFlag,
            THOST_FTDC_HFEN_Speculation, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_IOC, THOST_FTDC_VC_CV);
        int res = _tApi->ReqOrderInsert(&order, reqID);
        Lib::sysReqLog("T_OrderInsert", res);
    }

private:
    
    CThostFtdcTraderApi * _tApi;

    CThostFtdcInputOrderField createOrder(string exchangeID, string instrumnetID, 
        int isBuy, int total, double price, 
        // double stopPrice, 
        TThostFtdcOffsetFlagEnType offsetFlag, // 开平标志
        TThostFtdcHedgeFlagEnType hedgeFlag = THOST_FTDC_HFEN_Speculation, // 投机套保标志
        TThostFtdcOrderPriceTypeType priceType = THOST_FTDC_OPT_LimitPrice, // 报单价格条件
        TThostFtdcTimeConditionType timeCondition = THOST_FTDC_TC_IOC, // 有效期类型
        TThostFtdcVolumeConditionType volumeCondition = THOST_FTDC_VC_CV, //成交量类型
        TThostFtdcContingentConditionType contingentCondition = THOST_FTDC_CC_Immediately// 触发条件
        )
    {
        CThostFtdcInputOrderField order;

        strcpy(order.BrokerID, brokerID.c_str()); ///经纪公司代码
        strcpy(order.InvestorID, userID.c_str()); ///投资者代码
        strcpy(order.InstrumentID, instrumnetID.c_str()); ///合约代码
        strcpy(order.UserID, userID.c_str()); ///用户代码
        strcpy(order.ExchangeID, exchangeID.c_str()); ///交易所代码

        order.MinVolume = 1;///最小成交量
        order.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;///强平原因
        order.IsAutoSuspend = 0;///自动挂起标志
        order.UserForceClose = 0;///用户强评标志

        order.Direction = isBuy ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell; ///买卖方向
        order.VolumeTotalOriginal = total;///数量
        order.LimitPrice = price;///价格
        // order.StopPrice = stopPrice;///止损价

        ///组合开平标志
        //THOST_FTDC_OFEN_Open 开仓
        //THOST_FTDC_OFEN_Close 平仓
        //THOST_FTDC_OFEN_ForceClose 强平
        //THOST_FTDC_OFEN_CloseToday 平今
        //THOST_FTDC_OFEN_CloseYesterday 平昨
        //THOST_FTDC_OFEN_ForceOff 强减
        //THOST_FTDC_OFEN_LocalForceClose 本地强平
        order.CombOffsetFlag[0] = offsetFlag;
        if (THOST_FTDC_OFEN_ForceClose == offsetFlag) {
            order.ForceCloseReason = THOST_FTDC_FCC_Other; // 其他
            order.UserForceClose = 1;
        }

        ///组合投机套保标志
        // THOST_FTDC_HFEN_Speculation 投机
        // THOST_FTDC_HFEN_Arbitrage 套利
        // THOST_FTDC_HFEN_Hedge 套保
        order.CombHedgeFlag[0] = hedgeFlag;
        
        ///报单价格条件
        // THOST_FTDC_OPT_AnyPrice 任意价
        // THOST_FTDC_OPT_LimitPrice 限价
        // THOST_FTDC_OPT_BestPrice 最优价
        // THOST_FTDC_OPT_LastPrice 最新价
        // THOST_FTDC_OPT_LastPricePlusOneTicks 最新价浮动上浮1个ticks
        // THOST_FTDC_OPT_LastPricePlusTwoTicks 最新价浮动上浮2个ticks
        // THOST_FTDC_OPT_LastPricePlusThreeTicks 最新价浮动上浮3个ticks
        // THOST_FTDC_OPT_AskPrice1 卖一价
        // THOST_FTDC_OPT_AskPrice1PlusOneTicks 卖一价浮动上浮1个ticks
        // THOST_FTDC_OPT_AskPrice1PlusTwoTicks 卖一价浮动上浮2个ticks
        // THOST_FTDC_OPT_AskPrice1PlusThreeTicks 卖一价浮动上浮3个ticks
        // THOST_FTDC_OPT_BidPrice1 买一价
        // THOST_FTDC_OPT_BidPrice1PlusOneTicks 买一价浮动上浮1个ticks
        // THOST_FTDC_OPT_BidPrice1PlusTwoTicks 买一价浮动上浮2个ticks
        // THOST_FTDC_OPT_BidPrice1PlusThreeTicks 买一价浮动上浮3个ticks
        // THOST_FTDC_OPT_FiveLevelPrice 五档价
        order.OrderPriceType = priceType;

        ///有效期类型
        // THOST_FTDC_TC_IOC 立即完成，否则撤销
        // THOST_FTDC_TC_GFS 本节有效
        // THOST_FTDC_TC_GFD 当日有效
        // THOST_FTDC_TC_GTD 指定日期前有效
        // THOST_FTDC_TC_GTC 撤销前有效
        // THOST_FTDC_TC_GFA 集合竞价有效
        order.TimeCondition = timeCondition;

        ///成交量类型
        // THOST_FTDC_VC_AV 任何数量
        // THOST_FTDC_VC_MV 最小数量
        // THOST_FTDC_VC_CV 全部数量
        order.VolumeCondition = volumeCondition;
        
        ///触发条件
        // THOST_FTDC_CC_Immediately 立即
        // THOST_FTDC_CC_Touch 止损
        // THOST_FTDC_CC_TouchProfit 止赢
        // THOST_FTDC_CC_ParkedOrder 预埋单
        // THOST_FTDC_CC_LastPriceGreaterThanStopPrice 最新价大于条件价
        // THOST_FTDC_CC_LastPriceGreaterEqualStopPrice 最新价大于等于条件价
        // THOST_FTDC_CC_LastPriceLesserThanStopPrice 最新价小于条件价
        // THOST_FTDC_CC_LastPriceLesserEqualStopPrice 最新价小于等于条件价
        // THOST_FTDC_CC_AskPriceGreaterThanStopPrice 卖一价大于条件价
        // THOST_FTDC_CC_AskPriceGreaterEqualStopPrice 卖一价大于等于条件价
        // THOST_FTDC_CC_AskPriceLesserThanStopPrice 卖一价小于条件价
        // THOST_FTDC_CC_AskPriceLesserEqualStopPrice 卖一价小于等于条件价
        // THOST_FTDC_CC_BidPriceGreaterThanStopPrice 买一价大于条件价
        // THOST_FTDC_CC_BidPriceGreaterEqualStopPrice 买一价大于等于条件价
        // THOST_FTDC_CC_BidPriceLesserThanStopPrice 买一价小于条件价
        // THOST_FTDC_CC_BidPriceLesserEqualStopPrice 买一价小于等于条件价
        order.ContingentCondition = contingentCondition;

        ///报单引用
        orderRef++;
        sprintf(order.OrderRef, "%d", orderRef);

        ///请求编号
        reqID++;
        order.RequestID = reqID;
        
        // order.GTDDate = ;///GTD日期
        // order.BusinessUnit = ;///业务单元
        // order.IsSwapOrder = ;///互换单标志
        // order.InvestUnitID = ;///投资单元代码
        // order.AccountID = ;///资金账号
        // order.CurrencyID = ;///币种代码
        // order.ClientID = ;///交易编码
        // order.IPAddress = ;///IP地址
        // order.MacAddress = ;///Mac地址

        return order;
    }
    
};