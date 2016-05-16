#include "TraderSpi.h"
#include "../libs/Lib.h"

TraderSpi::TraderSpi(TradeSrv * service, string logPath)
{
    _service = service;
    _logPath = logPath;
    _sessionID = 0;
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

void TraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog(_logPath, "TradeSrv[OnRspSettlementInfoConfirm]", pRspInfo, nRequestID, bIsLast);
    // _service->getPosition();

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSrv[OnRspSettlementInfoConfirm]";
    if (pSettlementInfoConfirm) {
        info << "|ConfirmDate|" << pSettlementInfoConfirm->ConfirmDate;
        info << "|ConfirmTime|" << pSettlementInfoConfirm->ConfirmTime;
    }
    info << endl;
    info.close();
}

void TraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog(_logPath, "TradeSrv[onLogin]", pRspInfo, nRequestID, bIsLast);
    _service->onLogin(pRspUserLogin);
    _service->confirm();
    _sessionID = pRspUserLogin->SessionID;

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

    if (!pInvestorPosition) {
        _service->getPosition();
    }
    _service->onPositionRtn(pInvestorPosition);

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSrv[onPosition]";
    if (pInvestorPosition) {
        info << "|bIsLast|" << bIsLast;
        info << "|iID|" << pInvestorPosition->InstrumentID;
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

void TraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog(_logPath, "TradeSrv[onOrderInsert]", pRspInfo, nRequestID, bIsLast);
}

void TraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    if (_sessionID != pOrder->SessionID) return;
    _service->onOrderRtn(pOrder);
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSrv[onOrder]";
    if (pOrder) {
        info << "|iID|" << pOrder->InstrumentID;
        info << "|FrontID|" << pOrder->FrontID;
        info << "|SessionID|" << pOrder->SessionID;
        info << "|OrderRef|" << pOrder->OrderRef;
        info << "|OrderSysID|" << pOrder->OrderSysID;
        info << "|OrderSubmitStatus|" << pOrder->OrderSubmitStatus;
        info << "|OrderStatus|" << pOrder->OrderStatus;
        info << "|RelativeOrderSysID|" << pOrder->RelativeOrderSysID;
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
        info << "|iID|" << pTrade->InstrumentID;
        info << "|OrderRef|" << pTrade->OrderRef;
        info << "|TradeID|" << pTrade->TradeID;
        info << "|OrderSysID|" << pTrade->OrderSysID;
        info << "|OrderLocalID|" << pTrade->OrderLocalID;
        info << "|TradeDate|" << pTrade->TradeDate;
        info << "|TradeTime|" << pTrade->TradeTime;
        info << "|TradingDay|" << pTrade->TradingDay;
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
        info << "|OrderSysID|" << pInputOrderAction->OrderSysID;
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

