#include "MarketSpi.h"
#include "../iniReader/iniReader.h"
#include <string>

using namespace std;

int main(int argc, char const *argv[])
{
    // 初始化参数
    parseIniFile("sample.ini");
    string flowPath = getOptionToString("flow_path");
    string bid      = getOptionToString("market_broker_id");
    string userID   = getOptionToString("market_user_id");
    string password = getOptionToString("market_password");
    string mURL     = getOptionToString("market_front");

    // 初始化交易接口

    CThostFtdcMdApi * mApi = CThostFtdcMdApi::CreateFtdcMdApi(flowPath.c_str());
    MarketSpi mSpi(mApi, bid, userID, password); // 初始化回调实例
    mApi->RegisterSpi(&mSpi);
    mApi->RegisterFront(Lib::stoc(mURL));
    mApi->Init();
    mApi->Join();

    return 0;
}
