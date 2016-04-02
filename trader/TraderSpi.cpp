#include "TraderSpi.h"
#include "../lib.h"

TraderSpi::TraderSpi(CThostFtdcTraderApi * api, string brokerID, string userID, string password)
{
    _tApi = api;
    _brokerID = brokerID;
    _userID = userID;
    _password = password;
}

TraderSpi::~TraderSpi()
{
}

void TraderSpi::OnFrontConnected()
{
    CThostFtdcReqUserLoginField reqUserLogin;
    memset(&reqUserLogin, 0, sizeof(reqUserLogin));

    strcpy(reqUserLogin.BrokerID, _brokerID.c_str());
    strcpy(reqUserLogin.UserID, _userID.c_str());
    strcpy(reqUserLogin.Password, _password.c_str());

    // 发出登陆请求
    int res = _tApi->ReqUserLogin(&reqUserLogin, 0);
    Lib::sysReqLog("T_UserLogin", res);
}

void TraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    cout << "logined" << endl;
}
