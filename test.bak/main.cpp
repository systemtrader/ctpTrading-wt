#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include "QTradingService.h"
#include "config.h"
#include "lib.h"
#include <fstream>

using namespace std;

QTradingService * tService;

// 信号处理
void dealSignal(int sig)
{
    tService->stop();
    delete tService;
    exit(0);
}

//**********************************
// 根据argv[1]判断是实盘（0）还是仿真（1）
// main函数中初始化服务
// 监听信号进行手动操作
//**********************************
int main(int argc, char const *argv[])
{
    // 记录pid
    ofstream pid;
    string path = getPath("", PATH_PID);
    pid.open(path.c_str(), ios::out);
    pid << getpid();
    pid.close();

    // 初始化环境
    int env;
    if (argc == 1) {
        env = ENV_SIMU;
    } else {
        if (argv[1] == "0") env = ENV_REAL;
        else env = ENV_SIMU;
    }

    tService = new QTradingService(env);
    // 添加监听
    signal(SIG_STOP, dealSignal);
    // 服务启动，后期可以做成监听
    tService->start();

    return 0;
}


