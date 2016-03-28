#include "MarkDataSpi.h"
#include "lib.h"
#include <cstring>

MarkDataSpi::MarkDataSpi(CThostFtdcMdApi * mdApi)
{
    _mdApi = mdApi;

    string sysPath = getLogPath("sys"),
           markDataPath = getLogPath("markData");
    _sysLogger.open(sysPath.c_str(), ios::app);
    _markData.open(markDataPath.c_str(), ios::app);

    _userID = "";
    _password = "";
    _brokerID = "";
    _timestamp = "%Y-%m-%d %H:%M:%S";
}

void MarkDataSpi::OnFrontConnected()
{
    int res = _login();

    _sysReqRet("OnFrontConnected", "UserLogin", res);
}

int MarkDataSpi::_login()
{
    CThostFtdcReqUserLoginField reqUserLogin;

    memset(&reqUserLogin, 0, sizeof(reqUserLogin));

    strcpy(reqUserLogin.BrokerID, _brokerID.c_str());
    strcpy(reqUserLogin.UserID, _userID.c_str());
    strcpy(reqUserLogin.Password, _password.c_str());

    // 发出登陆请求
    int res = _mdApi->ReqUserLogin(&reqUserLogin, 0);
    return res;
}

void MarkDataSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _sysErrLog("OnRspError", pRspInfo, nRequestID, bIsLast);
}

void MarkDataSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _sysLogger << "OnRspUserLogin: ";

    _sysLogger << " LoginTime: " << pRspUserLogin->LoginTime;
    _sysLogger << " BrokerID: " << pRspUserLogin->BrokerID;
    _sysLogger << " UserID: " << pRspUserLogin->UserID;
    _sysLogger << " FrontID: " << pRspUserLogin->FrontID;
    _sysLogger << " SessionID: " << pRspUserLogin->SessionID;

    _sysLogger << endl;

    _sysErrLog("OnRspUserLogin", pRspInfo, nRequestID, bIsLast);

    char nicode[] = "ni1605";
    char * Instrumnet[]={nicode};
    int res = _mdApi->SubscribeMarketData (Instrumnet,1);
    _sysReqRet("OnRspUserLogin", "SubscribeMarketData", res);
}

void MarkDataSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _sysErrLog("OnRspSubMarketData", pRspInfo, nRequestID, bIsLast);
}

void MarkDataSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    _saveMarkData(pDepthMarketData);
}

void MarkDataSpi::_sysErrLog(string fun, CThostFtdcRspInfoField *info, int id, int isLast)
{
    _sysLogger << getDate(_timestamp);
    _sysLogger << " [ERROR]";
    _sysLogger << " | " << fun;
    _sysLogger << " | ErrCode : " << info->ErrorID;
    _sysLogger << " | ErrMsg : " << info->ErrorMsg;
    _sysLogger << " | RequestID : " << id;
    _sysLogger << " | IsLast : " << isLast;
    _sysLogger << endl;
}

void MarkDataSpi::_sysReqRet(string fun, string req, int code)
{
    _sysLogger << getDate(_timestamp);
    _sysLogger << " [REQUEST]";
    _sysLogger << " | " << fun;
    _sysLogger << " | " << req << " : " << code;
    _sysLogger << endl;
}

void MarkDataSpi::_saveMarkData(CThostFtdcDepthMarketDataField *data)
{
    _markData << getDate(_timestamp) << "|";
    _markData << data->TradingDay << "|";
    _markData << data->InstrumentID << "|";
    _markData << data->ExchangeID << "|";
    _markData << data->ExchangeInstID << "|";
    _markData << data->LastPrice << "|";
    _markData << data->PreSettlementPrice << "|";
    _markData << data->PreClosePrice << "|";
    _markData << data->PreOpenInterest << "|";
    _markData << data->OpenPrice << "|";
    _markData << data->HighestPrice << "|";
    _markData << data->LowestPrice << "|";
    _markData << data->Volume << "|";
    _markData << data->Turnover << "|";
    _markData << data->OpenInterest << "|";
    _markData << data->ClosePrice << "|";
    _markData << data->SettlementPrice << "|";
    _markData << data->UpperLimitPrice << "|";
    _markData << data->LowerLimitPrice << "|";
    _markData << data->PreDelta << "|";
    _markData << data->CurrDelta << "|";
    _markData << data->UpdateTime << "|";
    _markData << data->UpdateMillisec << "|";
    _markData << data->BidPrice1 << "|";
    _markData << data->BidVolume1 << "|";
    _markData << data->AskPrice1 << "|";
    _markData << data->AskVolume1 << "|";
    _markData << data->BidPrice2 << "|";
    _markData << data->BidVolume2 << "|";
    _markData << data->AskPrice2 << "|";
    _markData << data->AskVolume2 << "|";
    _markData << data->BidPrice3 << "|";
    _markData << data->BidVolume3 << "|";
    _markData << data->AskPrice3 << "|";
    _markData << data->AskVolume3 << "|";
    _markData << data->BidPrice4 << "|";
    _markData << data->BidVolume4 << "|";
    _markData << data->AskPrice4 << "|";
    _markData << data->AskVolume4 << "|";
    _markData << data->BidPrice5 << "|";
    _markData << data->BidVolume5 << "|";
    _markData << data->AskPrice5 << "|";
    _markData << data->AskVolume5 << "|";
    _markData << data->AveragePrice << "|";
    _markData << data->ActionDay << endl;
}

MarkDataSpi::~MarkDataSpi()
{
    _sysLogger.close();
}
