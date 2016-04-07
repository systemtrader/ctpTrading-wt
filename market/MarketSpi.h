#ifndef MARK_SPI_H
#define MARK_SPI_H

#include "../ThostFtdcMdApi.h"
#include "../ThostFtdcTraderApi.h"
#include "../libs/Lib.h"
#include <fstream>
#include <string>
#include <cstring>

using namespace std;

class MarketSpi : public CThostFtdcMdSpi
{
private:

    CThostFtdcMdApi * _mdApi;

    ofstream _marketData;

    string _brokerID;
    string _userID;
    string _password;
    int _flag;

    int _cfd;

    void _saveMarketData(CThostFtdcDepthMarketDataField *);

public:

    MarketSpi(CThostFtdcMdApi *, int, string, string, string);
    ~MarketSpi();

    void OnFrontConnected();
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

    void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
};


#endif
