#include "QTradingService.h"
#include "config.h"
#include "lib.h"
#include <iostream>
#include <cstring>

QTradingService::QTradingService(int env)
{
    _env = env;
}

QTradingService::~QTradingService()
{
    cout << "~" << endl;
}

void QTradingService::start()
{
    // 初始化变量
    if (_env == ENV_REAL) {
        _mdURL      = MARKETDATA_FRONT_API_REAL;
        _mdBrokerID = MARKETDATA_BROKER_ID_REAL;
        _mdUserID   = MARKETDATA_USER_ID_REAL;
        _mdPassword = MARKETDATA_PASSWORD_REAL;

        _tURL      = TRADER_FRONT_API_REAL;
        _tBrokerID = TRADER_BROKER_ID_REAL;
        _tUserID   = TRADER_USER_ID_REAL;
        _tPassword = TRADER_PASSWORD_REAL;

    } else {
        _mdURL      = MARKETDATA_FRONT_API_SIMU;
        _mdBrokerID = MARKETDATA_BROKER_ID_SIMU;
        _mdUserID   = MARKETDATA_USER_ID_SIMU;
        _mdPassword = MARKETDATA_PASSWORD_SIMU;

        _tURL      = TRADER_FRONT_API_SIMU;
        _tBrokerID = TRADER_BROKER_ID_SIMU;
        _tUserID   = TRADER_USER_ID_SIMU;
        _tPassword = TRADER_PASSWORD_SIMU;
    }

    // 初始化交易接口
    _mdApi = CThostFtdcMdApi::CreateFtdcMdApi(FLOW_PATH.c_str());
    _tApi = CThostFtdcTraderApi::CreateFtdcTraderApi(FLOW_PATH.c_str());

    _mdSpi = new MarketDataSpi(_mdApi, _mdBrokerID, _mdUserID, _mdPassword); // 初始化回调实例
    _tSpi = new TraderSpi(_tApi, _tBrokerID, _tUserID, _tPassword);

    _mdApi->RegisterSpi(_mdSpi);
    _tApi->RegisterSpi(_tSpi);

    _mdApi->RegisterFront(stoc(_mdURL.c_str()));
    _mdApi->Init();
    _mdApi->Join();

    _tApi->SubscribePrivateTopic(THOST_TERT_RESUME);
    _tApi->SubscribePublicTopic(THOST_TERT_RESUME);
    _tApi->RegisterFront(stoc(_tURL.c_str()));
    _tApi->Init();
    _tApi->Join();

}

void QTradingService::stop()
{
    cout << "stop" << endl;

    if (_tApi) {
        _tApi->RegisterSpi(NULL);
        _tApi->Release();
        _tApi = NULL;
    }

    if (_tSpi) {
        delete _tSpi;
        _tSpi = NULL;
    }

    if (_mdApi) {
        _mdApi->RegisterSpi(NULL);
        _mdApi->Release();
        _mdApi = NULL;
    }

    if (_mdSpi) {
        delete _mdSpi;
        _mdSpi = NULL;
    }

}
