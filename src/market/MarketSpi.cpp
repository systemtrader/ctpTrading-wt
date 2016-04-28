#include "MarketSpi.h"

using namespace std;

MarketSpi::MarketSpi(CThostFtdcMdApi * mdApi, string logPath,
    int serviceID,
    string brokerID, string userID, string password, string instrumnetID, int db)
{
    _mdApi = mdApi;
    _logPath = logPath;

    _userID = userID;
    _password = password;
    _brokerID = brokerID;
    _instrumnetID = instrumnetID;

    _klineClient = new QClient(serviceID, sizeof(MSG_TO_KLINE));
    _store = new Redis("127.0.0.1", 6379, db);

}

MarketSpi::~MarketSpi()
{
    _mdApi = NULL;
    // delete _store;
    // delete _klineClient;
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
    Lib::sysReqLog(_logPath, "MarketSrv[ReqUserLogin]", res);
}


void MarketSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog(_logPath, "MarketSrv[OnRspUserLogin]", pRspInfo, nRequestID, bIsLast);

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "MarketSrv[LoginSuccess]";
    if (pRspUserLogin) {
        info << "|SessionID|" << pRspUserLogin->SessionID;
        info << "|TradingDay|" << pRspUserLogin->TradingDay;
        info << "|MaxOrderRef|" << pRspUserLogin->MaxOrderRef;
    }
    info << endl;
    info.close();

    char * Instrumnet[]={Lib::stoc(_instrumnetID)};
    int res = _mdApi->SubscribeMarketData (Instrumnet, 1);
    Lib::sysReqLog(_logPath, "MarketSrv[SubscribeMarketData]", res);
}

void MarketSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog(_logPath, "MarketSrv[OnRspSubMarketData]", pRspInfo, nRequestID, bIsLast);
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
    Lib::sysErrLog(_logPath, "MarketSrv[OnRspError]", pRspInfo, nRequestID, bIsLast);
}

void MarketSpi::_saveMarketData(CThostFtdcDepthMarketDataField *data)
{
    if (!data) return;
    // 发送给K线系统
    MSG_TO_KLINE msg = {0};
    msg.msgType = MSG_TICK;
    msg.tick.price = data->LastPrice;
    msg.tick.volume = data->Volume;
    msg.tick.bidPrice1 = data->BidPrice1;
    msg.tick.askPrice1 = data->AskPrice1;
    strcpy(msg.tick.date, data->TradingDay);
    strcpy(msg.tick.time, data->UpdateTime);
    msg.tick.msec = data->UpdateMillisec;
    _klineClient->send((void *)&msg);


    // 将数据放入队列，以便存入DB
    string tickStr = Lib::tickData2String(msg.tick);
    string keyQ = "MARKET_TICK_Q";
    string keyD = "CURRENT_TICK";
    _store->set(keyD, tickStr); // tick数据，供全局使用
    _store->push(keyQ, tickStr);


    // 日志记录
    ofstream marketData;
    Lib::initMarketLogHandle(_logPath, marketData);
    marketData << data->TradingDay << "|";
    marketData << data->InstrumentID << "|";
    marketData << data->ExchangeID << "|";
    marketData << data->ExchangeInstID << "|";
    marketData << data->LastPrice << "|";
    marketData << data->PreSettlementPrice << "|";
    marketData << data->PreClosePrice << "|";
    marketData << data->PreOpenInterest << "|";
    marketData << data->OpenPrice << "|";
    marketData << data->HighestPrice << "|";
    marketData << data->LowestPrice << "|";
    marketData << data->Volume << "|";
    marketData << data->Turnover << "|";
    marketData << data->OpenInterest << "|";
    marketData << data->ClosePrice << "|";
    marketData << data->SettlementPrice << "|";
    marketData << data->UpperLimitPrice << "|";
    marketData << data->LowerLimitPrice << "|";
    marketData << data->PreDelta << "|";
    marketData << data->CurrDelta << "|";
    marketData << data->UpdateTime << "|";
    marketData << data->UpdateMillisec << "|";
    marketData << data->BidPrice1 << "|";
    marketData << data->BidVolume1 << "|";
    marketData << data->AskPrice1 << "|";
    marketData << data->AskVolume1 << "|";
    marketData << data->BidPrice2 << "|";
    marketData << data->BidVolume2 << "|";
    marketData << data->AskPrice2 << "|";
    marketData << data->AskVolume2 << "|";
    marketData << data->BidPrice3 << "|";
    marketData << data->BidVolume3 << "|";
    marketData << data->AskPrice3 << "|";
    marketData << data->AskVolume3 << "|";
    marketData << data->BidPrice4 << "|";
    marketData << data->BidVolume4 << "|";
    marketData << data->AskPrice4 << "|";
    marketData << data->AskVolume4 << "|";
    marketData << data->BidPrice5 << "|";
    marketData << data->BidVolume5 << "|";
    marketData << data->AskPrice5 << "|";
    marketData << data->AskVolume5 << "|";
    marketData << data->AveragePrice << "|";
    marketData << data->ActionDay << endl;
    marketData.close();
}


