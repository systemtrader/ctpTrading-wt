#include "MarketSpi.h"

using namespace std;

MarketSpi::MarketSpi(CThostFtdcMdApi * mdApi, string logPath,
    string brokerID, string userID, string password, string instrumnetIDs, int db)
{
    _mdApi = mdApi;
    _logPath = logPath;

    _userID = userID;
    _password = password;
    _brokerID = brokerID;

    _instrumnetIDs = Lib::split(instrumnetIDs, "/");
    _store = new Redis("127.0.0.1", 6379, db);

}

MarketSpi::~MarketSpi()
{
    _mdApi = NULL;
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

    int cnt = _instrumnetIDs.size();
    char ** Instrumnet;
    Instrumnet = (char**)malloc((sizeof(char*)) * cnt);
    for (int i = 0; i < cnt; i++) {
        char * tmp = Lib::stoc(_instrumnetIDs[i]);
        Instrumnet[i] = tmp;
    }

    int res = _mdApi->SubscribeMarketData (Instrumnet, cnt);
    Lib::sysReqLog(_logPath, "MarketSrv[SubscribeMarketData]", res);
    free(Instrumnet);
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

    TickData tick = {0};
    tick.price = data->LastPrice;
    tick.volume = data->Volume;
    tick.bidPrice1 = data->BidPrice1;
    tick.bidVolume1 = data->BidVolume1;
    tick.askPrice1 = data->AskPrice1;
    tick.askVolume1 = data->AskVolume1;
    strcpy(tick.date, data->ActionDay);
    strcpy(tick.time, data->UpdateTime);
    strcpy(tick.instrumnetID, data->InstrumentID);
    tick.msec = data->UpdateMillisec;

    // 将数据放入队列，以便存入DB
    string tickStr = Lib::tickData2String(tick);
    string keyQ = "MARKET_TICK_Q";
    _store->push(keyQ, tickStr);

}


