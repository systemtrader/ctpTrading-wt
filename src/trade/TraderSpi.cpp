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
    _service->getPositionDetail();

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSrv[onPosition]";
    if (pInvestorPosition) {
        info << "|bIsLast|" << bIsLast;
        info << "|PosiDirection|" << pInvestorPosition->PosiDirection;
        info << "|PositionDate|" << pInvestorPosition->PositionDate;
        info << "|YdPosition|" << pInvestorPosition->YdPosition;
        info << "|Position|" << pInvestorPosition->Position;
        info << "|OpenVolume|" << pInvestorPosition->OpenVolume;
        info << "|CloseVolume|" << pInvestorPosition->CloseVolume;
        info << "|TodayPosition|" << pInvestorPosition->TodayPosition;
    }
    info << endl;
    info.close();
}

void TraderSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog(_logPath, "TradeSrv[onPositionDetail]", pRspInfo, nRequestID, bIsLast);
    _service->onPositionDetailRtn(pInvestorPositionDetail);

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSrv[onPosition]";
    if (pInvestorPositionDetail) {
info << "|InstrumentID|" << pInvestorPositionDetail->InstrumentID;
info << "|BrokerID|" << pInvestorPositionDetail->BrokerID;
info << "|InvestorID|" << pInvestorPositionDetail->InvestorID;
info << "|HedgeFlag|" << pInvestorPositionDetail->HedgeFlag;
info << "|Direction|" << pInvestorPositionDetail->Direction;
info << "|OpenDate|" << pInvestorPositionDetail->OpenDate;
info << "|TradeID|" << pInvestorPositionDetail->TradeID;
info << "|Volume|" << pInvestorPositionDetail->Volume;
info << "|OpenPrice|" << pInvestorPositionDetail->OpenPrice;
info << "|TradingDay|" << pInvestorPositionDetail->TradingDay;
info << "|SettlementID|" << pInvestorPositionDetail->SettlementID;
info << "|TradeType|" << pInvestorPositionDetail->TradeType;
info << "|CombInstrumentID|" << pInvestorPositionDetail->CombInstrumentID;
info << "|ExchangeID|" << pInvestorPositionDetail->ExchangeID;
info << "|CloseProfitByDate|" << pInvestorPositionDetail->CloseProfitByDate;
info << "|CloseProfitByTrade|" << pInvestorPositionDetail->CloseProfitByTrade;
info << "|PositionProfitByDate|" << pInvestorPositionDetail->PositionProfitByDate;
info << "|PositionProfitByTrade|" << pInvestorPositionDetail->PositionProfitByTrade;
info << "|Margin|" << pInvestorPositionDetail->Margin;
info << "|ExchMargin|" << pInvestorPositionDetail->ExchMargin;
info << "|MarginRateByMoney|" << pInvestorPositionDetail->MarginRateByMoney;
info << "|MarginRateByVolume|" << pInvestorPositionDetail->MarginRateByVolume;
info << "|LastSettlementPrice|" << pInvestorPositionDetail->LastSettlementPrice;
info << "|SettlementPrice|" << pInvestorPositionDetail->SettlementPrice;
info << "|CloseVolume|" << pInvestorPositionDetail->CloseVolume;
info << "|CloseAmount|" << pInvestorPositionDetail->CloseAmount;
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

