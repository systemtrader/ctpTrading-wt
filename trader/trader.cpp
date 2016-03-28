#include "TraderSpi.h"
#include "../ThostFtdcTraderApi.h"
#include <iostream>

int main(int argc, char const *argv[])
{

    const char * version = CThostFtdcTraderApi::GetApiVersion();
    cout << version << endl;

    CThostFtdcTraderApi * traderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
    TraderSpi traderSpi(traderApi, userID, password, brokerID);

    traderApi->RegisterSpi(&traderSpi);
    traderApi->SubscribePrivateTopic(THOST_TERT_RESUME);
    traderApi->SubscribePublicTopic(THOST_TERT_RESUME);

    traderApi->RegisterFront(frontApi);
    traderApi->Init();
    traderApi->Join();
    traderApi->Release();

    return 0;
}
