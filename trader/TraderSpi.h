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

    string _userID;
    string _password;
    string _brokerID;

public:
    TraderSpi(CThostFtdcTraderApi *, string, string, string);
    ~TraderSpi();

    void OnFrontConnected();
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

};

#endif
