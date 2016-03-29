#ifndef QTRADING_SERVICE_H
#define QTRADING_SERVICE_H

#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include "MarketDataSpi.h"
#include "TraderSpi.h"
#include <fstream>
#include <string>
using namespace std;

class QTradingService
{
private:

    int _env;
    CThostFtdcMdApi * _mdApi;
    CThostFtdcTraderApi * _tApi;

    MarketDataSpi * _mdSpi;
    TraderSpi * _tSpi;

    string _mdURL, _mdUserID, _mdBrokerID, _mdPassword;
    string _tURL, _tUserID, _tBrokerID, _tPassword;

public:

    QTradingService(int env);
    ~QTradingService();

    void start(); // 开始服务
    void stop(); // 停止服务

};

#endif
