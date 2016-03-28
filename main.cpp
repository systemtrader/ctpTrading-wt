#include <iostream>
#include "ThostFtdcMdApi.h"
#include "MarkDataSpi.h"

using namespace std;

int main(int argc, char const *argv[])
{
    const char * str = CThostFtdcMdApi::GetApiVersion();
    cout << str << endl;


    // 初始化行情API实例
    CThostFtdcMdApi * mdApi = CThostFtdcMdApi::CreateFtdcMdApi("./flow/");
    // 初始化回调实例
    MarkDataSpi mdSpi(mdApi);
    mdApi->RegisterSpi(&mdSpi);
    char api[] = "tcp://180.168.146.187:10010";
    mdApi->RegisterFront(api);
    mdApi->Init();
    mdApi->Join();
    mdApi->Release();


    return 0;
}
