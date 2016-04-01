#include "MarketDataSpi.h"
#include "lib.h"
#include "config.h"
#include <cstring>

MarketDataSpi::MarketDataSpi(CThostFtdcMdApi * mdApi,
    // CThostFtdcTraderApi * tApi,
    string brokerID, string userID, string password)
{
    _mdApi = mdApi;
    // _tApi = tApi;

    string marketDataPath = getPath("market_data", PATH_DATA);
    _marketData.open(marketDataPath.c_str(), ios::app);

    _userID = userID;
    _password = password;
    _brokerID = brokerID;
}

void MarketDataSpi::OnFrontConnected()
{
    CThostFtdcReqUserLoginField reqUserLogin;

    memset(&reqUserLogin, 0, sizeof(reqUserLogin));

    strcpy(reqUserLogin.BrokerID, _brokerID.c_str());
    strcpy(reqUserLogin.UserID, _userID.c_str());
    strcpy(reqUserLogin.Password, _password.c_str());

    // 发出登陆请求
    int res = _mdApi->ReqUserLogin(&reqUserLogin, 0);
    sysReqLog("MD_UserLogin", res);
}


void MarketDataSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    sysErrLog("MD_UserLogin", pRspInfo, nRequestID, bIsLast);

    char * Instrumnet[]={stoc(INSTRUMENT_ID)};
    int res = _mdApi->SubscribeMarketData (Instrumnet, 1);
    sysReqLog("MD_SubscribeMarketData", res);
}

void MarketDataSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    sysErrLog("MD_SubMarketData", pRspInfo, nRequestID, bIsLast);
}

/**
 * 接收市场数据
 * 绘制K线图
 * 执行策略进行买卖
 * @param pDepthMarketData [description]
 */
void MarketDataSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    _saveMarketData(pDepthMarketData);
}

void MarketDataSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    sysErrLog("MD_RspError", pRspInfo, nRequestID, bIsLast);
}

void MarketDataSpi::_saveMarketData(CThostFtdcDepthMarketDataField *data)
{
    _marketData << getDate(LOG_TIMESTAMP_FORMAT) << LOG_SPLIT;
    _marketData << data->TradingDay << LOG_SPLIT;
    _marketData << data->InstrumentID << LOG_SPLIT;
    _marketData << data->ExchangeID << LOG_SPLIT;
    _marketData << data->ExchangeInstID << LOG_SPLIT;
    _marketData << data->LastPrice << LOG_SPLIT;
    _marketData << data->PreSettlementPrice << LOG_SPLIT;
    _marketData << data->PreClosePrice << LOG_SPLIT;
    _marketData << data->PreOpenInterest << LOG_SPLIT;
    _marketData << data->OpenPrice << LOG_SPLIT;
    _marketData << data->HighestPrice << LOG_SPLIT;
    _marketData << data->LowestPrice << LOG_SPLIT;
    _marketData << data->Volume << LOG_SPLIT;
    _marketData << data->Turnover << LOG_SPLIT;
    _marketData << data->OpenInterest << LOG_SPLIT;
    _marketData << data->ClosePrice << LOG_SPLIT;
    _marketData << data->SettlementPrice << LOG_SPLIT;
    _marketData << data->UpperLimitPrice << LOG_SPLIT;
    _marketData << data->LowerLimitPrice << LOG_SPLIT;
    _marketData << data->PreDelta << LOG_SPLIT;
    _marketData << data->CurrDelta << LOG_SPLIT;
    _marketData << data->UpdateTime << LOG_SPLIT;
    _marketData << data->UpdateMillisec << LOG_SPLIT;
    _marketData << data->BidPrice1 << LOG_SPLIT;
    _marketData << data->BidVolume1 << LOG_SPLIT;
    _marketData << data->AskPrice1 << LOG_SPLIT;
    _marketData << data->AskVolume1 << LOG_SPLIT;
    _marketData << data->BidPrice2 << LOG_SPLIT;
    _marketData << data->BidVolume2 << LOG_SPLIT;
    _marketData << data->AskPrice2 << LOG_SPLIT;
    _marketData << data->AskVolume2 << LOG_SPLIT;
    _marketData << data->BidPrice3 << LOG_SPLIT;
    _marketData << data->BidVolume3 << LOG_SPLIT;
    _marketData << data->AskPrice3 << LOG_SPLIT;
    _marketData << data->AskVolume3 << LOG_SPLIT;
    _marketData << data->BidPrice4 << LOG_SPLIT;
    _marketData << data->BidVolume4 << LOG_SPLIT;
    _marketData << data->AskPrice4 << LOG_SPLIT;
    _marketData << data->AskVolume4 << LOG_SPLIT;
    _marketData << data->BidPrice5 << LOG_SPLIT;
    _marketData << data->BidVolume5 << LOG_SPLIT;
    _marketData << data->AskPrice5 << LOG_SPLIT;
    _marketData << data->AskVolume5 << LOG_SPLIT;
    _marketData << data->AveragePrice << LOG_SPLIT;
    _marketData << data->ActionDay << endl;
}

MarketDataSpi::~MarketDataSpi()
{
    _marketData.close();

    // _tApi->RegisterSpi(NULL);
    // _tApi->Release();
    // _tApi = NULL;

}
