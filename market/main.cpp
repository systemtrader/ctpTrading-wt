#include "MarketSpi.h"
#include "../iniReader/iniReader.h"
#include <string>
#include <errno.h>
#include <iostream>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

CThostFtdcMdApi * mApi;
int cfd;

void shutdown(int sig)
{
    mApi->Release();
    close(cfd);
}

int main(int argc, char const *argv[])
{
    // 初始化参数
    parseIniFile("../config.ini");
    string flowPath = getOptionToString("flow_path");
    string bid      = getOptionToString("market_broker_id");
    string userID   = getOptionToString("market_user_id");
    string password = getOptionToString("market_password");
    string mURL     = getOptionToString("market_front");

    int          traderSrvPort = getOptionToInt("trader_srv_port");
    const char * traderSrvIp   = getOptionToChar("trader_srv_ip");

    signal(30, shutdown);
    cout << "kill -30 " << getpid() << endl;

    // init socket
    struct sockaddr_in serverAddr;

    if ((cfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "create socket error" << endl;
        exit(0);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(traderSrvPort);
    if (inet_pton(AF_INET, traderSrvIp, &serverAddr.sin_addr) <= 0) {
        cout << "inet_pton error" << endl;
        exit(0);
    }

    if (connect(cfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        cout << "connect error" << strerror(errno) << endl;
        exit(0);
    }

    // 初始化交易接口
    mApi = CThostFtdcMdApi::CreateFtdcMdApi(flowPath.c_str());
    MarketSpi mSpi(mApi, cfd, bid, userID, password); // 初始化回调实例
    mApi->RegisterSpi(&mSpi);
    mApi->RegisterFront(Lib::stoc(mURL));
    mApi->Init();
    mApi->Join();

    return 0;
}
