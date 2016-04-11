#include "MarketSpi.h"
#include "../iniReader/iniReader.h"
#include "../libs/Lib.h"
#include "../libs/Socket.h"
#include "../cmd.h"
#include <iostream>

using namespace std;

MarketSpi::MarketSpi(CThostFtdcMdApi * mdApi, int cfd,
    string brokerID, string userID, string password)
{
    _mdApi = mdApi;

    string marketDataPath = Lib::getPath("market_data", PATH_DATA);
    _marketData.open(marketDataPath.c_str(), ios::app);

    _userID = userID;
    _password = password;
    _brokerID = brokerID;

    _cfd = cfd;
    _flag = 1;

    _store = new Redis("127.0.0.1", 6379, 1);

}

MarketSpi::~MarketSpi()
{
    _marketData.close();
    _mdApi = NULL;
    delete _store;
    cout << "~MarketSpi" << endl;
}

void MarketSpi::OnFrontConnected()
{

    CThostFtdcReqUserLoginField reqUserLogin;

    memset(&reqUserLogin, 0, sizeof(reqUserLogin));

    strcpy(reqUserLogin.BrokerID, _brokerID.c_str());
    strcpy(reqUserLogin.UserID, _userID.c_str());
    strcpy(reqUserLogin.Password, _password.c_str());

    // 发出登陆请求
    int res = _mdApi->ReqUserLogin(&reqUserLogin, 0);
    Lib::sysReqLog("M_ReqUserLogin", res);
}


void MarketSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog("M_OnRspUserLogin", pRspInfo, nRequestID, bIsLast);

    ofstream info;
    Lib::initInfoLogHandle(info);
    info << "MD_LoginSuccess" << "|";
    info << "SessionID" << "|" << pRspUserLogin->SessionID << "|";
    info << "TradingDay" << "|" << pRspUserLogin->TradingDay << "|";
    info << "MaxOrderRef" << "|" << pRspUserLogin->MaxOrderRef << endl;
    info.close();

    string instrumnetID = getOptionToString("instrumnet_id");
    char * Instrumnet[]={Lib::stoc(instrumnetID)};
    int res = _mdApi->SubscribeMarketData (Instrumnet, 1);
    Lib::sysReqLog("M_SubscribeMarketData", res);
}

void MarketSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog("M_OnRspSubMarketData", pRspInfo, nRequestID, bIsLast);
}

/**
 * 接收市场数据
 * 绘制K线图
 * 执行策略进行买卖
 * @param pDepthMarketData [description]
 */
void MarketSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    _saveMarketData(pDepthMarketData);
}

void MarketSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog("M_OnRspError", pRspInfo, nRequestID, bIsLast);
}

void MarketSpi::_saveMarketData(CThostFtdcDepthMarketDataField *data)
{
    string localTime = Lib::getDate("%Y-%m-%d %H:%M:%S");
    string storeData = localTime + "_" +
                  string(data->TradingDay) + "_" +
                  string(data->UpdateTime) + "_" +
                  Lib::dtos(data->LastPrice) + "_" +
                  Lib::itos(data->Volume);
    // 发送给K线系统
    // string cmd = CMD_MSG_TICK + "_" + storeData;
    // sendMsg(_cfd, cmd);

    // 将数据放入队列，以便存入DB
    string keyQ = "MARKET_TICK_Q";
    string keyD = "CURRENT_TICK";
    _store->set(keyD, storeData); // tick数据，供全局使用
    _store->push(keyQ, storeData);

    // 日志记录
    _marketData << localTime << "|";
    _marketData << data->TradingDay << "|";
    _marketData << data->InstrumentID << "|";
    _marketData << data->ExchangeID << "|";
    _marketData << data->ExchangeInstID << "|";
    _marketData << data->LastPrice << "|";
    _marketData << data->PreSettlementPrice << "|";
    _marketData << data->PreClosePrice << "|";
    _marketData << data->PreOpenInterest << "|";
    _marketData << data->OpenPrice << "|";
    _marketData << data->HighestPrice << "|";
    _marketData << data->LowestPrice << "|";
    _marketData << data->Volume << "|";
    _marketData << data->Turnover << "|";
    _marketData << data->OpenInterest << "|";
    _marketData << data->ClosePrice << "|";
    _marketData << data->SettlementPrice << "|";
    _marketData << data->UpperLimitPrice << "|";
    _marketData << data->LowerLimitPrice << "|";
    _marketData << data->PreDelta << "|";
    _marketData << data->CurrDelta << "|";
    _marketData << data->UpdateTime << "|";
    _marketData << data->UpdateMillisec << "|";
    _marketData << data->BidPrice1 << "|";
    _marketData << data->BidVolume1 << "|";
    _marketData << data->AskPrice1 << "|";
    _marketData << data->AskVolume1 << "|";
    _marketData << data->BidPrice2 << "|";
    _marketData << data->BidVolume2 << "|";
    _marketData << data->AskPrice2 << "|";
    _marketData << data->AskVolume2 << "|";
    _marketData << data->BidPrice3 << "|";
    _marketData << data->BidVolume3 << "|";
    _marketData << data->AskPrice3 << "|";
    _marketData << data->AskVolume3 << "|";
    _marketData << data->BidPrice4 << "|";
    _marketData << data->BidVolume4 << "|";
    _marketData << data->AskPrice4 << "|";
    _marketData << data->AskVolume4 << "|";
    _marketData << data->BidPrice5 << "|";
    _marketData << data->BidVolume5 << "|";
    _marketData << data->AskPrice5 << "|";
    _marketData << data->AskVolume5 << "|";
    _marketData << data->AveragePrice << "|";
    _marketData << data->ActionDay << endl;

}


