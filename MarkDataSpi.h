#ifndef MARK_DATA_SPI_H
#define MARK_DATA_SPI_H

#include "ThostFtdcMdApi.h"
#include <fstream>
#include <string>

using namespace std;

class MarkDataSpi : public CThostFtdcMdSpi
{
private:
    CThostFtdcMdApi * _mdApi;
    ofstream _sysLogger;
    ofstream _markData;
    string _userID;
    string _password;
    string _brokerID;
    string _timestamp;
    string _formatMarkData(CThostFtdcDepthMarketDataField);
    void _sysErrLog(string, CThostFtdcRspInfoField *, int, int);
    void _sysReqRet(string, string, int);
    void _saveMarkData(CThostFtdcDepthMarketDataField *);
    int _login();
public:
    MarkDataSpi(CThostFtdcMdApi * mdApi);
    ~MarkDataSpi();
    void OnFrontConnected();
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, 
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, 
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);
    void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
};


#endif