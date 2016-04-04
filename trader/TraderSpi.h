#ifndef TRADER_SPI_H
#define TRADER_SPI_H

#include "../ThostFtdcTraderApi.h"
#include <fstream>
#include <string>
#include <cstring>

using namespace std;

class TraderSpi : public CThostFtdcTraderSpi
{
private:

    CThostFtdcTraderApi * _tApi;

public:
    TraderSpi(CThostFtdcTraderApi *);
    ~TraderSpi();

    void OnFrontConnected();
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, 
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    ///错误应答
    void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    ///报单通知
    void OnRtnOrder(CThostFtdcOrderField *pOrder);
    ///成交通知
    void OnRtnTrade(CThostFtdcTradeField *pTrade);
    ///报单录入错误回报
    void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);

};

#endif
