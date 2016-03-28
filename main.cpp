#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include "QTradingService.h"

using namespace std;

QTradingService * tService;

// 信号处理
void dealSignal(int sig)
{
    switch (sig) {
        case SIG_STOP:
            tService->stop();
            delete tService;
            exit(0);
            break;
        default:
            break;
    }
}

//**********************************
// 根据argv[1]判断是实盘（0）还是仿真（1）
// main函数中初始化服务
// 监听信号进行手动操作
//**********************************
int main(int argc, char const *argv[])
{
    // 初始化环境
    int env;
    if (argc == 1) {
        env = ENV_SIMU;
    } else {
        if (argv[1] == "0") env = ENV_REAL;
        else env = ENV_SIMU;
    }

    tService = new QTradingService(env);
    tService->init();
    // 添加监听
    signal(SIG_STOP, dealSignal);

    tService->start();

    // const char * str = CThostFtdcMdApi::GetApiVersion();
    // cout << str << endl;

    // // 初始化行情API实例
    // mdApi = CThostFtdcMdApi::CreateFtdcMdApi("./flow/");
    // // 初始化回调实例
    // MarkDataSpi mdSpi(mdApi);
    // mdApi->RegisterSpi(&mdSpi);
    // char api[] = "tcp://180.168.146.187:10010";
    // mdApi->RegisterFront(api);
    // mdApi->Init();
    // mdApi->Join();
    // mdApi->Release();

    return 0;
}


