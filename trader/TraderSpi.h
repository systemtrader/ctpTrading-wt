#ifndef MARK_DATA_SPI_H
#define MARK_DATA_SPI_H

#include "../ThostFtdcTraderApi.h"
#include <fstream>
#include <string>

using namespace std;

class TraderSpi : public CThostFtdcTraderSpi
{
private:
    CThostFtdcTraderApi * _traderApi;

    string _userID;
    string _password;
    string _brokerID;

    ofstream _sysLogger;
    ofstream _markData;
    string _timestampFormat;

    void _saveMarkData(CThostFtdcDepthMarketDataField *data);

public:
    TraderSpi(CThostFtdcTraderApi *, string, string, string);
    ~TraderSpi();

    void OnFrontConnected();
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

};

#endif
