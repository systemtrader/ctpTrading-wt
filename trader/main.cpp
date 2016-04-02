#include "TraderSpi.h"
#include "../iniReader/iniReader.h"
#include "../lib.h"
#include "../cmd.h"
#include <string>
#include <iostream>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

CThostFtdcTraderApi * tApi;

bool action(string msg);
void actionShutDown();

int main(int argc, char const *argv[])
{
    // 初始化参数
    parseIniFile("../config.ini");
    string flowPath = getOptionToString("flow_path");
    string bid      = getOptionToString("trader_broker_id");
    string userID   = getOptionToString("trader_user_id");
    string password = getOptionToString("trader_password");
    string tURL     = getOptionToString("trader_front");

    int traderSrvPort = getOptionToInt("trader_srv_port");

    // 初始化交易接口
    tApi = CThostFtdcTraderApi::CreateFtdcTraderApi(flowPath.c_str());
    TraderSpi tSpi(tApi, bid, userID, password); // 初始化回调实例
    tApi->RegisterSpi(&tSpi);
    tApi->RegisterFront(Lib::stoc(tURL));
    tApi->Init();
    // tApi->Join();
    
    // 服务化
    int sfd, cfd;
    struct sockaddr_in serverAddr;

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cout << "create socket error" << endl;
        exit(0);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(traderSrvPort);

    if (bind(sfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        cout << "bind socket error" << endl;
        exit(0);
    }

    if (listen(sfd, 10) == -1) {
        cout << "listen socket error" << endl;
        exit(0);
    }

    char buff[1024];
    string msg;
    int n;
    while (true) {
        cout << "while" << endl;
        if ((cfd = accept(sfd, (struct sockaddr *)NULL, NULL)) == -1) {
            cout << "accept socket error: " << strerror(errno) <<  endl;
            continue;
        }
        n = recv(cfd, buff, 1024, 0);
        close(cfd);
        buff[n] = '\0';
        msg = string(buff);
        msg = trim(msg);
        if (action(msg)) {
            break;
        }
    }
    close(sfd);

    return 0;
}

bool action(string msg)
{
    cout << msg << endl;
    if (msg.compare(CMD_MSG_SHUTDOWN)) {
        actionShutDown();
        return false;
    }
    return true;
}

void actionShutDown()
{
    tApi->Release();
}
