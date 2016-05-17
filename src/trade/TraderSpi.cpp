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


// info << "|BrokerID|" << pOrder->BrokerID;
// info << "|InvestorID|" << pOrder->InvestorID;
// info << "|InstrumentID|" << pOrder->InstrumentID;
// info << "|OrderRef|" << pOrder->OrderRef;
// info << "|UserID|" << pOrder->UserID;
// info << "|OrderPriceType|" << pOrder->OrderPriceType;
// info << "|Direction|" << pOrder->Direction;
// info << "|CombOffsetFlag|" << pOrder->CombOffsetFlag;
// info << "|CombHedgeFlag|" << pOrder->CombHedgeFlag;
// info << "|LimitPrice|" << pOrder->LimitPrice;
// info << "|VolumeTotalOriginal|" << pOrder->VolumeTotalOriginal;
// info << "|TimeCondition|" << pOrder->TimeCondition;
// info << "|GTDDate|" << pOrder->GTDDate;
// info << "|VolumeCondition|" << pOrder->VolumeCondition;
// info << "|MinVolume|" << pOrder->MinVolume;
// info << "|ContingentCondition|" << pOrder->ContingentCondition;
// info << "|StopPrice|" << pOrder->StopPrice;
// info << "|ForceCloseReason|" << pOrder->ForceCloseReason;
// info << "|IsAutoSuspend|" << pOrder->IsAutoSuspend;
// info << "|BusinessUnit|" << pOrder->BusinessUnit;
// info << "|RequestID|" << pOrder->RequestID;
// info << "|OrderLocalID|" << pOrder->OrderLocalID;
// info << "|ExchangeID|" << pOrder->ExchangeID;
// info << "|ParticipantID|" << pOrder->ParticipantID;
// info << "|ClientID|" << pOrder->ClientID;
// info << "|ExchangeInstID|" << pOrder->ExchangeInstID;
// info << "|TraderID|" << pOrder->TraderID;
// info << "|InstallID|" << pOrder->InstallID;
// info << "|OrderSubmitStatus|" << pOrder->OrderSubmitStatus;
// info << "|NotifySequence|" << pOrder->NotifySequence;
// info << "|TradingDay|" << pOrder->TradingDay;
// info << "|SettlementID|" << pOrder->SettlementID;
// info << "|OrderSysID|" << pOrder->OrderSysID;
// info << "|OrderSource|" << pOrder->OrderSource;
// info << "|OrderStatus|" << pOrder->OrderStatus;
// info << "|OrderType|" << pOrder->OrderType;
// info << "|VolumeTraded|" << pOrder->VolumeTraded;
// info << "|VolumeTotal|" << pOrder->VolumeTotal;
// info << "|InsertDate|" << pOrder->InsertDate;
// info << "|InsertTime|" << pOrder->InsertTime;
// info << "|ActiveTime|" << pOrder->ActiveTime;
// info << "|SuspendTime|" << pOrder->SuspendTime;
// info << "|UpdateTime|" << pOrder->UpdateTime;
// info << "|CancelTime|" << pOrder->CancelTime;
// info << "|ActiveTraderID|" << pOrder->ActiveTraderID;
// info << "|ClearingPartID|" << pOrder->ClearingPartID;
// info << "|SequenceNo|" << pOrder->SequenceNo;
// info << "|FrontID|" << pOrder->FrontID;
// info << "|SessionID|" << pOrder->SessionID;
// info << "|UserProductInfo|" << pOrder->UserProductInfo;
// info << "|StatusMsg|" << pOrder->StatusMsg;
// info << "|UserForceClose|" << pOrder->UserForceClose;
// info << "|ActiveUserID|" << pOrder->ActiveUserID;
// info << "|BrokerOrderSeq|" << pOrder->BrokerOrderSeq;
// info << "|RelativeOrderSysID|" << pOrder->RelativeOrderSysID;
// info << "|ZCETotalTradedVolume|" << pOrder->ZCETotalTradedVolume;
// info << "|IsSwapOrder|" << pOrder->IsSwapOrder;
// info << "|BranchID|" << pOrder->BranchID;
// info << "|InvestUnitID|" << pOrder->InvestUnitID;
// info << "|AccountID|" << pOrder->AccountID;
// info << "|CurrencyID|" << pOrder->CurrencyID;
// info << "|IPAddress|" << pOrder->IPAddress;
// info << "|MacAddress|" << pOrder->MacAddress;

    }
    info << endl;
    info.close();
    _service->onOrderRtn(pOrder);
}

void TraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSrv[onTrade]";
    if (pTrade) {
        info << "|iID|" << pTrade->InstrumentID;
        info << "|OrderRef|" << pTrade->OrderRef;
        info << "|TradeID|" << pTrade->TradeID;
        info << "|OrderSysID|" << pTrade->OrderSysID;
        info << "|Price|" << pTrade->Price;
        info << "|OrderLocalID|" << pTrade->OrderLocalID;
        info << "|TradeDate|" << pTrade->TradeDate;
        info << "|TradeTime|" << pTrade->TradeTime;
        info << "|TradingDay|" << pTrade->TradingDay;

//         info << "|BrokerID|" << pTrade->BrokerID;
// info << "|InvestorID|" << pTrade->InvestorID;
// info << "|InstrumentID|" << pTrade->InstrumentID;
// info << "|OrderRef|" << pTrade->OrderRef;
// info << "|UserID|" << pTrade->UserID;
// info << "|ExchangeID|" << pTrade->ExchangeID;
// info << "|TradeID|" << pTrade->TradeID;
// info << "|Direction|" << pTrade->Direction;
// info << "|OrderSysID|" << pTrade->OrderSysID;
// info << "|ParticipantID|" << pTrade->ParticipantID;
// info << "|ClientID|" << pTrade->ClientID;
// info << "|TradingRole|" << pTrade->TradingRole;
// info << "|ExchangeInstID|" << pTrade->ExchangeInstID;
// info << "|OffsetFlag|" << pTrade->OffsetFlag;
// info << "|HedgeFlag|" << pTrade->HedgeFlag;
// info << "|Volume|" << pTrade->Volume;
// info << "|TradeDate|" << pTrade->TradeDate;
// info << "|TradeTime|" << pTrade->TradeTime;
// info << "|TradeType|" << pTrade->TradeType;
// info << "|PriceSource|" << pTrade->PriceSource;
// info << "|TraderID|" << pTrade->TraderID;
// info << "|OrderLocalID|" << pTrade->OrderLocalID;
// info << "|ClearingPartID|" << pTrade->ClearingPartID;
// info << "|BusinessUnit|" << pTrade->BusinessUnit;
// info << "|SequenceNo|" << pTrade->SequenceNo;
// info << "|TradingDay|" << pTrade->TradingDay;
// info << "|SettlementID|" << pTrade->SettlementID;
// info << "|BrokerOrderSeq|" << pTrade->BrokerOrderSeq;
// info << "|TradeSource|" << pTrade->TradeSource;
    }
    info << endl;
    info.close();
    _service->onTraded(pTrade);

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

