#include "TraderSpi.h"
#include "lib.h"
#include <cstring>

TraderSpi::TraderSpi(CThostFtdcTraderApi * api)
{
    _traderApi = api;
    _timestampFormat = "%Y-%m-%d %H:%M:%S";

    string sysPath = getLogPath("trade_sys"),
           markDataPath = getLogPath("trade_markData");
    _sysLogger.open(sysPath.c_str(), ios::app);
    _markData.open(markDataPath.c_str(), ios::app);

}

TraderSpi::~TraderSpi()
{
    _sysLogger.close();
    _markData.close();
}

void TraderSpi::OnFrontConnected()
{
    CThostFtdcReqUserLoginField reqUserLogin;

    memset(&reqUserLogin, 0, sizeof(reqUserLogin));

    strcpy(reqUserLogin.BrokerID, _brokerID.c_str());
    strcpy(reqUserLogin.UserID, _userID.c_str());
    strcpy(reqUserLogin.Password, _password.c_str());

    // 发出登陆请求
    _traderApi->ReqUserLogin(&reqUserLogin, 0);
}

void TraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _sysLogger << "OnRspUserLogin: ";

    _sysLogger << " LoginTime: " << pRspUserLogin->LoginTime;
    _sysLogger << " BrokerID: " << pRspUserLogin->BrokerID;
    _sysLogger << " UserID: " << pRspUserLogin->UserID;
    _sysLogger << " FrontID: " << pRspUserLogin->FrontID;
    _sysLogger << " SessionID: " << pRspUserLogin->SessionID;

    _sysLogger << endl;

    CThostFtdcQryDepthMarketDataField md;
    memset(&md, 0, sizeof(md));
    char code[] = "ni1605";
    strcpy(md.InstrumentID, code);
    _traderApi->ReqQryDepthMarketData(&md, 1);

}

void TraderSpi::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _saveMarkData(pDepthMarketData);
}

void TraderSpi::_saveMarkData(CThostFtdcDepthMarketDataField *data)
{
    _markData << getDate(_timestampFormat) << "|";
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
