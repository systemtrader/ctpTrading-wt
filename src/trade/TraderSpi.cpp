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
    _service->setFrontID(pRspUserLogin->FrontID);
    _service->setSessionID(pRspUserLogin->SessionID);
    _service->setMaxOrderRef(atoi(pRspUserLogin->MaxOrderRef));
    _service->getPosition();

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSrv[onLogin]" << "|";
    info << "SessionID" << "|" << pRspUserLogin->SessionID << "|";
    info << "TradingDay" << "|" << pRspUserLogin->TradingDay << "|";
    info << "MaxOrderRef" << "|" << pRspUserLogin->MaxOrderRef << endl;
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
        info << "YdPosition" << "|" << pInvestorPosition->YdPosition << "|";
        info << "Position" << "|" << pInvestorPosition->Position << "|";
        info << "OpenVolume" << "|" << pInvestorPosition->OpenVolume << "|";
        info << "CloseVolume" << "|" << pInvestorPosition->CloseVolume;
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
    info << "TradeSrv[onOrder]" << "|";
    info << "FrontID" << "|" << pOrder->FrontID << "|";
    info << "SessionID" << "|" << pOrder->SessionID << "|";
    info << "OrderRef" << "|" << pOrder->OrderRef << "|";
    info << "OrderSubmitStatus" << "|" << pOrder->OrderSubmitStatus << endl;
    info.close();

}

void TraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    _service->onTraded(pTrade);
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSrv[onTrade]" << "|";
    info << "TradeID" << "|" << pTrade->TradeID << endl;
    info.close();

}

void TraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _service->onCancel(pInputOrderAction);
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSrv[onTrade]" << "|";
    // info << "TradeID" << "|" << pInputOrderAction->TradeID << endl;
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

