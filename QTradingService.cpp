#include "QTradingService.h"
#include "MarkDataSpi.h"
#include "TraderSpi.h"
#include "config.h"
#include "lib.h"
#include <iostream>

using namespace std;

QTradingService::QTradingService(int env)
{
    _env = env;
    cout << "constructor" << endl;
}

QTradingService::~QTradingService()
{
    cout << "~" << endl;
}

void QTradingService::init()
{
    cout << "init" << endl;

    // 初始化行情接口
    _mdApi = CThostFtdcMdApi::CreateFtdcMdApi(FLOW_PATH.c_str());
    MarkDataSpi mdSpi(_mdApi); // 初始化回调实例
    _mdApi->RegisterSpi(&mdSpi);
    string mdURL = _env == ENV_REAL ? MARKDATA_FRONT_API_REAL : MARKDATA_FRONT_API_SIMU;
    _mdApi->RegisterFront(stoc(mdURL.c_str()));
    _mdApi->Init();

    // 初始化交易接口
    _tApi = CThostFtdcTraderApi::CreateFtdcTraderApi(FLOW_PATH.c_str());
    TraderSpi tSpi(_tApi);
    _tApi->RegisterSpi(&tSpi);
    _tApi->SubscribePrivateTopic(THOST_TERT_RESUME);
    _tApi->SubscribePublicTopic(THOST_TERT_RESUME);
    string tURL = _env == ENV_REAL ? TRADER_FRONT_API_REAL : TRADER_FRONT_API_SIMU; 
    _tApi->RegisterFront(stoc(tURL.c_str()));
    _tApi->Init();
}

void QTradingService::start()
{
    cout << "start..." << endl;
    while (true) {
        cout << "wait..." << endl;
        sleep(1);
    }
}

void QTradingService::stop()
{
    cout << "stop" << endl;
}
