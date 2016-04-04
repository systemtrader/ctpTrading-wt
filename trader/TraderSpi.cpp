#include "TraderSpi.h"
#include "global.h"
#include "../lib.h"

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
    Lib::sysReqLog("T_UserLogin", res);
}

void TraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog("T_UserLoginRsp", pRspInfo, nRequestID, bIsLast);
    if (pRspUserLogin) {
        frontID = pRspUserLogin->FrontID;
        sessionID = pRspUserLogin->SessionID;
        orderRef = atoi(pRspUserLogin->MaxOrderRef);
    } else {
        exit(-1);
    }
    cout << "logined" << endl;
}

void TraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, 
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog("T_OrderInsertRsp", pRspInfo, nRequestID, bIsLast);
}

void TraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    ofstream info;
    Lib::initInfoLogHandle(info);
    info << "TradeOrder" << "|";
    info << "FrontID" << "|" << pOrder->FrontID << "|";
    info << "SessionID" << "|" << pOrder->SessionID << "|";
    info << "OrderRef" << "|" << pOrder->OrderRef << "|";
    info << "OrderSubmitStatus" << "|" << pOrder->OrderSubmitStatus << "|";
    info.close();

}

void TraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    ofstream info;
    Lib::initInfoLogHandle(info);
    info << "TradeSuccess" << "|";
    info << "TradeID" << "|" << pTrade->TradeID << "|";
    info.close();

}

void TraderSpi::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, 
    CThostFtdcRspInfoField *pRspInfo)
{
    Lib::sysErrLog("T_OrderInsertRtnErr", pRspInfo, 0, 1);
}

void TraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog("T_ReqErr", pRspInfo, nRequestID, bIsLast);
}

