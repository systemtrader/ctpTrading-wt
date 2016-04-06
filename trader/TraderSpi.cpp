#include "TraderSpi.h"
#include "../lib.h"

extern string brokerID;
extern string userID;
extern string password;

extern int reqID;
extern int orderRef;

extern TThostFtdcFrontIDType   frontID;
extern TThostFtdcSessionIDType sessionID;


TraderSpi::TraderSpi(CThostFtdcTraderApi * api)
{
    _tApi = api;
}

TraderSpi::~TraderSpi()
{
    _tApi = NULL;
    cout << "~TraderSpi" << endl;
}

void TraderSpi::OnFrontConnected()
{
    CThostFtdcReqUserLoginField reqUserLogin;
    memset(&reqUserLogin, 0, sizeof(reqUserLogin));

    strcpy(reqUserLogin.BrokerID, brokerID.c_str());
    strcpy(reqUserLogin.UserID, userID.c_str());
    strcpy(reqUserLogin.Password, password.c_str());

    // 发出登陆请求
    int res = _tApi->ReqUserLogin(&reqUserLogin, 0);
    Lib::sysReqLog("T_ReqUserLogin", res);
}

void TraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog("T_OnRspUserLogin", pRspInfo, nRequestID, bIsLast);
    if (pRspUserLogin) {
        frontID = pRspUserLogin->FrontID;
        sessionID = pRspUserLogin->SessionID;
        orderRef = atoi(pRspUserLogin->MaxOrderRef);
        ofstream info;
        Lib::initInfoLogHandle(info);
        info << "T_OnRspUserLogin" << "|";
        info << "SessionID" << "|" << sessionID << "|";
        info << "TradingDay" << "|" << pRspUserLogin->TradingDay << "|";
        info << "MaxOrderRef" << "|" << pRspUserLogin->MaxOrderRef << endl;
        info.close();

        CThostFtdcSettlementInfoConfirmField ef = {0};
        strcpy(ef.BrokerID, brokerID.c_str());
        strcpy(ef.InvestorID, userID.c_str());
        strcpy(ef.ConfirmDate, pRspUserLogin->TradingDay);
        // strcpy(ef.InvestorID, userID.c_str());
        int res = _tApi->ReqSettlementInfoConfirm(&ef, 0);
        Lib::sysReqLog("T_ReqSettlementInfoConfirm", res);
    }
}

void TraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    ofstream info;
    Lib::initInfoLogHandle(info);
    info << "T_OnRspSettlementInfoConfirm" << "|";
    info << "ConfirmDate" << "|" << pSettlementInfoConfirm->ConfirmDate << "|";
    info << "ConfirmTime" << "|" << pSettlementInfoConfirm->ConfirmTime << endl;
    info.close();
};

void TraderSpi::OnRspQryExchange(CThostFtdcExchangeField *pExchange,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    // ofstream info;
    // Lib::initInfoLogHandle(info);
    // info << "T_OnRspQryExchange" << "|";
    // info << "ExchangeID" << "|" << pExchange->ExchangeID << "|";
    // info << "ExchangeProperty" << "|" << pExchange->ExchangeProperty << "|";
    // info << "ExchangeName" << "|" << pExchange->ExchangeName << endl;
    // info.close();
}

void TraderSpi::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    // ofstream info;
    // Lib::initInfoLogHandle(info);
    // info << "T_OnRspQryDepthMarketData" << "|";
    // info << "ExchangeID" << "|" << pDepthMarketData->ExchangeID << endl;
    // info.close();
}

void TraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog("T_OnRspOrderInsert", pRspInfo, nRequestID, bIsLast);
}

void TraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    ofstream info;
    Lib::initInfoLogHandle(info);
    info << "TradeOrder" << "|";
    info << "FrontID" << "|" << pOrder->FrontID << "|";
    info << "SessionID" << "|" << pOrder->SessionID << "|";
    info << "OrderRef" << "|" << pOrder->OrderRef << "|";
    info << "OrderSubmitStatus" << "|" << pOrder->OrderSubmitStatus << endl;
    info.close();

}

void TraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    ofstream info;
    Lib::initInfoLogHandle(info);
    info << "TradeSuccess" << "|";
    info << "TradeID" << "|" << pTrade->TradeID << endl;
    info.close();

}

void TraderSpi::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder,
    CThostFtdcRspInfoField *pRspInfo)
{
    Lib::sysErrLog("T_OnErrRtnOrderInsert", pRspInfo, 0, 1);
}

void TraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog("T_OnRspError", pRspInfo, nRequestID, bIsLast);
}

