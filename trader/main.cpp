#include "TraderSpi.h"
#include "TradeAction.h"
#include "../iniReader/iniReader.h"
#include "../libs/Lib.h"
#include "../libs/Socket.h"
#include "../cmd.h"
#include <string>
#include <iostream>

using namespace std;

CThostFtdcTraderApi * tApi;
TradeAction * tAction;

string brokerID;
string userID;
string password;

int reqID = 0;
int orderRef = 0;

TThostFtdcFrontIDType   frontID;
TThostFtdcSessionIDType sessionID;


bool action(string msg, std::vector<string>);

int main(int argc, char const *argv[])
{
    // 初始化参数
    parseIniFile("config.ini");
    string flowPath   = getOptionToString("flow_path");
    string tURL       = getOptionToString("trader_front");
    int traderSrvPort = getOptionToInt("trader_srv_port");

    brokerID        = getOptionToString("trader_broker_id");
    userID          = getOptionToString("trader_user_id");
    password        = getOptionToString("trader_password");


    // 初始化交易接口
    tApi = CThostFtdcTraderApi::CreateFtdcTraderApi(flowPath.c_str());
    TraderSpi tSpi(tApi); // 初始化回调实例
    tApi->RegisterSpi(&tSpi);
    // tApi->SubscribePrivateTopic(THOST_TERT_QUICK);
    // tApi->SubscribePublicTopic(THOST_TERT_QUICK);
    tApi->SubscribePrivateTopic(THOST_TERT_RESUME);
    tApi->SubscribePublicTopic(THOST_TERT_RESUME);

    tApi->RegisterFront(Lib::stoc(tURL));
    tApi->Init();
    // tApi->Join();

    // 初始化交易行为
    tAction = new TradeAction(tApi);

    // 服务化
    int sfd;
    sfd = getSSocket(traderSrvPort);

    char buff[1024];
    string msgLine, msg;
    vector<string> params;
    int cfd, n;
    cout << "TradeSrv start success!" << endl;
    while (true) {
        if ((cfd = accept(sfd, (struct sockaddr *)NULL, NULL)) == -1) {
            cout << "accept socket error: " << strerror(errno) <<  endl;
            continue;
        }
        n = recv(cfd, buff, 1024, 0);
        close(cfd);
        if (n == 0) continue;

        buff[n] = '\0';
        msgLine = string(buff);
        msgLine = trim(msgLine);
        params = Lib::split(msgLine, "_");
        msg = params[0];
        if (action(msg, params)) {
            break;
        }
    }
    close(sfd);

    return 0;
}

bool action(string msg, std::vector<string> params)
{
    cout << "MSG:" << msg << endl;
    if (msg.compare(CMD_MSG_SHUTDOWN) == 0) {
        delete tAction;
        tApi->Release();
        return true;
    }
    if (msg.compare(CMD_MSG_TRADE_FOK) == 0) {
        string instrumnetID = params[1];
        int isBuy = atoi(params[2].c_str());
        int total = atoi(params[3].c_str());
        double price = atof(params[4].c_str());
        int offsetType = atoi(params[5].c_str());
        cout  << instrumnetID << "|" << isBuy << "|" << total << "|" << price << "|" << offsetType << endl;
        tAction->tradeFOK(instrumnetID, isBuy, total, price, offsetType);
    }
    return false;
}

