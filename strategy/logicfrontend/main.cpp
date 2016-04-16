#include "TradeLogic.h"
#include "../../iniReader/iniReader.h"
#include "../../libs/Socket.h"
#include "../../libs/Lib.h"
#include "../../cmd.h"
#include <vector>

TradeLogic * service;

bool action(string msg, std::vector<string>);

int main(int argc, char const *argv[])
{
    // 初始化参数
    parseIniFile("config.ini");
    int openKCountMax = getOptionToInt("open_k_count_max");
    int openKCountMin = getOptionToInt("open_k_count_min");
    int openKCountMean = getOptionToInt("open_k_count_mean");
    int kRang = getOptionToInt("k_range");
    int sellCloseKLineNum = getOptionToInt("");
    int buyCloseKLineNum = getOptionToInt("");

    int logicFrontSrvPort = getOptionToInt("logic_front_srv_port");

    service = new TradeLogic(openKCountMax, openKCountMin, openKCountMean, kRang, sellCloseKLineNum, buyCloseKLineNum);

    // service->onKLineOpen();
    // return 0;

    // 服务化
    int sfd = getSSocket(logicFrontSrvPort);

    char buff[1024];
    string msgLine, msg;
    vector<string> params;
    int cfd, n;
    cout << "TradeLogic frontend start success!" << endl;
    if ((cfd = accept(sfd, (struct sockaddr *)NULL, NULL)) == -1) {
        cout << "accept socket error: " << strerror(errno) <<  endl;
    }
    while ((n = recv(cfd, buff, 1024, 0)) > 0) {
        buff[n] = '\0';
        msgLine = string(buff);
        msgLine = trim(msgLine);
        params = Lib::split(msgLine, "_");
        msg = params[0];
        if (action(msg, params)) {
            break;
        }
    }
    close(cfd);
    close(sfd);
    cout << "TradeLogic frontend stop success!" << endl;
    return 0;
}

bool action(string msg, std::vector<string> params)
{
    cout << "MSG:" << msg << endl;
    if (msg.compare(CMD_MSG_SHUTDOWN) == 0) {
        if (service) delete service;
        return true;
    }
    if (msg.compare(CMD_MSG_KLINE_OPEN) == 0) {
        service->onKLineOpen();
    }
    if (msg.compare(CMD_MSG_KLINE_CLOSE) == 0) {
        KLineBlock block = KLineBlock::makeSimple(params[2], params[3], params[6],
            params[7], params[8], params[9], params[10]);
        block.show();
        service->onKLineClose(block);
    }
    return false;
}

