#include "MarketSpi.h"
#include "../iniReader/iniReader.h"
#include "../socket.h"
#include <string>
#include <iostream>
#include <signal.h>

using namespace std;

CThostFtdcMdApi * mApi;
int cfd;

void shutdown(int sig)
{
    mApi->Release();
    close(cfd);
    string path = Lib::getPath("", PATH_PID);
    remove(path.c_str());
}

int main(int argc, char const *argv[])
{
    // 初始化参数
    parseIniFile("config.ini");
    string flowPath = getOptionToString("flow_path");
    string bid      = getOptionToString("market_broker_id");
    string userID   = getOptionToString("market_user_id");
    string password = getOptionToString("market_password");
    string mURL     = getOptionToString("market_front");

    int          kLineSrvPort = getOptionToInt("k_line_srv_port");
    const char * kLineSrvIp   = getOptionToChar("k_line_srv_ip");

    signal(30, shutdown);
    ofstream pid;
    string path = Lib::getPath("", PATH_PID);
    pid.open(path.c_str(), ios::out);
    pid << getpid();
    pid.close();

    // init socket
    cfd = getCSocket(kLineSrvIp, kLineSrvPort);
    // cfd = 0;

    // 初始化交易接口
    mApi = CThostFtdcMdApi::CreateFtdcMdApi(flowPath.c_str());
    MarketSpi mSpi(mApi, cfd, bid, userID, password); // 初始化回调实例
    mApi->RegisterSpi(&mSpi);
    mApi->RegisterFront(Lib::stoc(mURL));
    mApi->Init();
    cout << "MarketSrv start success!" << endl;
    mApi->Join();

    return 0;
}
