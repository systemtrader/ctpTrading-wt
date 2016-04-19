#include "TradeStrategy.h"
#include "../../iniReader/iniReader.h"
#include "../../libs/Socket.h"
#include "../../libs/Lib.h"
#include "../../cmd.h"
#include <vector>

TradeStrategy * service;
int timeoutSec;

bool action(string msg, std::vector<string>);

int main(int argc, char const *argv[])
{
    // 初始化参数
    parseIniFile("config.ini");
    timeoutSec = getOptionToInt("trade_timeout_sec");
    int tradeBackendSrvPort = getOptionToInt("trade_backend_srv_port");

    service = new TradeStrategy();

    // 服务化
    int sfd = getSSocket(tradeBackendSrvPort);

    char buff[1024];
    string msgLine, msg;
    vector<string> params;
    int cfd, n;
    cout << "Trade backend start success!" << endl;
    while (true) {
        if ((cfd = accept(sfd, (struct sockaddr *)NULL, NULL)) == -1) {
            cout << "accept socket error: " << strerror(errno) <<  endl;
            continue;
        }
        if ((n = recv(cfd, buff, 1024, 0)) <= 0) continue;
        buff[n] = '\0';
        msgLine = string(buff);
        msgLine = trim(msgLine);
        params = Lib::split(msgLine, "_");
        msg = params[0];
        if (action(msg, params)) {
            break;
        }
        close(cfd);
    }
    close(sfd);
    cout << "Trade backend stop success!" << endl;
    return 0;
}

bool action(string msg, std::vector<string> params)
{
    cout << "MSG:" << msg << endl;
    if (msg.compare(CMD_MSG_SHUTDOWN) == 0) {
        if (service) delete service;
        return true;
    }
    // 下单操作
    if (msg.compare(CMD_MSG_TRADE_BUYOPEN) == 0) {
        service->tradeAction(TRADE_ACTION_BUYOPEN, Lib::stod(params[1]), 1);
    }
    if (msg.compare(CMD_MSG_TRADE_SELLOPEN) == 0) {
        service->tradeAction(TRADE_ACTION_SELLOPEN, Lib::stod(params[1]), 1);
    }
    if (msg.compare(CMD_MSG_TRADE_SELLCLOSE) == 0) {
        service->tradeAction(TRADE_ACTION_SELLCLOSE, Lib::stod(params[1]), 1);
    }
    if (msg.compare(CMD_MSG_TRADE_BUYCLOSE) == 0) {
        service->tradeAction(TRADE_ACTION_BUYCLOSE, Lib::stod(params[1]), 1);
    }

    // 下单回馈
    if (msg.compare(CMD_MSG_ON_TRADED) == 0) {
        service->onTradeMsgBack(true);
    }
    if (msg.compare(CMD_MSG_ON_CANCELED) == 0) {
        service->onTradeMsgBack(false);
    }
    return false;
}

