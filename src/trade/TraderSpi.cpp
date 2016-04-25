#include "TraderSpi.h"
#include "../libs/Lib.h"

TraderSpi::TraderSpi(TradeSrv * service, string logPath)
{
    _service = service;
    _logPath = logPath;
}

TraderSpi::~TraderSpi()
{
    _service = NULL;
    cout << "~TraderSpi" << endl;
}

void TraderSpi::OnFrontConnected()
{
    _service->login();
}

void TraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog(_logPath, "TradeSrv[onLogin]", pRspInfo, nRequestID, bIsLast);
    _service->onLogin(pRspUserLogin);
    _service->getPosition();

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSrv[onLogin]";
    if (pRspUserLogin) {
        info << "|SessionID|" << pRspUserLogin->SessionID;
        info << "|FrontID|" << pRspUserLogin->FrontID;
        info << "|TradingDay|" << pRspUserLogin->TradingDay;
        info << "|MaxOrderRef|" << pRspUserLogin->MaxOrderRef;
    }
    info << endl;
    info.close();
}

void TraderSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog(_logPath, "TradeSrv[onPosition]", pRspInfo, nRequestID, bIsLast);
    _service->onPositionRtn(pInvestorPosition);

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSrv[onPosition]" << "|";
    if (pInvestorPosition) {
        info << "|bIsLast|" << bIsLast;
info << "|InstrumentID|" << pInvestorPosition->InstrumentID;
info << "|BrokerID|" << pInvestorPosition->BrokerID;
info << "|InvestorID|" << pInvestorPosition->InvestorID;
info << "|PosiDirection|" << pInvestorPosition->PosiDirection;
info << "|HedgeFlag|" << pInvestorPosition->HedgeFlag;
info << "|PositionDate|" << pInvestorPosition->PositionDate;
info << "|YdPosition|" << pInvestorPosition->YdPosition;
info << "|Position|" << pInvestorPosition->Position;
info << "|LongFrozen|" << pInvestorPosition->LongFrozen;
info << "|ShortFrozen|" << pInvestorPosition->ShortFrozen;
info << "|LongFrozenAmount|" << pInvestorPosition->LongFrozenAmount;
info << "|ShortFrozenAmount|" << pInvestorPosition->ShortFrozenAmount;
info << "|OpenVolume|" << pInvestorPosition->OpenVolume;
info << "|CloseVolume|" << pInvestorPosition->CloseVolume;
info << "|OpenAmount|" << pInvestorPosition->OpenAmount;
info << "|CloseAmount|" << pInvestorPosition->CloseAmount;
info << "|PositionCost|" << pInvestorPosition->PositionCost;
info << "|PreMargin|" << pInvestorPosition->PreMargin;
info << "|UseMargin|" << pInvestorPosition->UseMargin;
info << "|FrozenMargin|" << pInvestorPosition->FrozenMargin;
info << "|FrozenCash|" << pInvestorPosition->FrozenCash;
info << "|FrozenCommission|" << pInvestorPosition->FrozenCommission;
info << "|CashIn|" << pInvestorPosition->CashIn;
info << "|Commission|" << pInvestorPosition->Commission;
info << "|CloseProfit|" << pInvestorPosition->CloseProfit;
info << "|PositionProfit|" << pInvestorPosition->PositionProfit;
info << "|PreSettlementPrice|" << pInvestorPosition->PreSettlementPrice;
info << "|SettlementPrice|" << pInvestorPosition->SettlementPrice;
info << "|TradingDay|" << pInvestorPosition->TradingDay;
info << "|SettlementID|" << pInvestorPosition->SettlementID;
info << "|OpenCost|" << pInvestorPosition->OpenCost;
info << "|ExchangeMargin|" << pInvestorPosition->ExchangeMargin;
info << "|CombPosition|" << pInvestorPosition->CombPosition;
info << "|CombLongFrozen|" << pInvestorPosition->CombLongFrozen;
info << "|CombShortFrozen|" << pInvestorPosition->CombShortFrozen;
info << "|CloseProfitByDate|" << pInvestorPosition->CloseProfitByDate;
info << "|CloseProfitByTrade|" << pInvestorPosition->CloseProfitByTrade;
info << "|TodayPosition|" << pInvestorPosition->TodayPosition;
info << "|MarginRateByMoney|" << pInvestorPosition->MarginRateByMoney;
info << "|MarginRateByVolume|" << pInvestorPosition->MarginRateByVolume;
info << "|StrikeFrozen|" << pInvestorPosition->StrikeFrozen;
info << "|StrikeFrozenAmount|" << pInvestorPosition->StrikeFrozenAmount;
info << "|AbandonFrozen|" << pInvestorPosition->AbandonFrozen;

    }

    info << endl;
    info.close();

}

void TraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog(_logPath, "TradeSrv[onOrderInsert]", pRspInfo, nRequestID, bIsLast);
}

void TraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    _service->onOrderRtn(pOrder);
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSrv[onOrder]";
    if (pOrder) {
        info << "|FrontID|" << pOrder->FrontID;
        info << "|SessionID|" << pOrder->SessionID;
        info << "|OrderRef|" << pOrder->OrderRef;
        info << "|OrderSysID|" << pOrder->OrderSysID;
        info << "|OrderSubmitStatus|" << pOrder->OrderSubmitStatus;
        info << "|OrderStatus|" << pOrder->OrderStatus;
    }
    info << endl;
    info.close();
}

void TraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    _service->onTraded(pTrade);
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSrv[onTrade]";
    if (pTrade) {
        info << "|OrderRef|" << pTrade->OrderRef;
        info << "|TradeID|" << pTrade->TradeID;
        info << "|OrderLocalID|" << pTrade->OrderLocalID;
    }
    info << endl;
    info.close();

}

void TraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog(_logPath, "TradeSrv[onOrderAction]", pRspInfo, nRequestID, bIsLast);
    _service->onCancel(pInputOrderAction);

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSrv[onOrderAction]";
    if (pInputOrderAction) {
        info << "|OrderRef|" << pInputOrderAction->OrderRef;
        info << "|OrderActionRef|" << pInputOrderAction->OrderActionRef;
        info << "|SessionID|" << pInputOrderAction->SessionID;
    }
    info << endl;
    info.close();
}

void TraderSpi::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder,
    CThostFtdcRspInfoField *pRspInfo)
{
    Lib::sysErrLog(_logPath, "TradeSrv[ErrRtnOrderInsert]", pRspInfo, 0, 1);
}

void TraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog(_logPath, "TradeSrv[RspError]", pRspInfo, nRequestID, bIsLast);
}

