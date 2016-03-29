#ifndef MARK_DATA_SPI_H
#define MARK_DATA_SPI_H

#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#include <fstream>
#include <string>

using namespace std;

class MarketDataSpi : public CThostFtdcMdSpi
{
private:

    CThostFtdcMdApi * _mdApi;
    // CThostFtdcTraderApi * _tApi;

    ofstream _marketData;

    string _brokerID;
    string _userID;
    string _password;

    void _saveMarketData(CThostFtdcDepthMarketDataField *);

public:

    MarketDataSpi(CThostFtdcMdApi *, string, string, string);
    ~MarketDataSpi();

    void OnFrontConnected();
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);
    // void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout,
    //     CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
};


#endif
