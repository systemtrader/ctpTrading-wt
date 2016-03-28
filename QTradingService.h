#ifndef QTRADING_SERVICE_H
#define QTRADING_SERVICE_H

#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include "config.h"

class QTradingService
{
private:

    CThostFtdcMdApi * _mdApi;
    CThostFtdcTraderApi * _tApi;
    int _env;

public:

    QTradingService(int env);
    ~QTradingService();

    void init(); // 初始化服务
    void start(); // 开始服务
    void stop(); // 停止服务

};

#endif
