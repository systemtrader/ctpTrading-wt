#include "cmd.h"
#include "iniReader/iniReader.h"
#include "Libs/Lib.h"
#include "Libs/Socket.h"
#include <string>
#include <iostream>
#include <signal.h>
#include <fstream>

using namespace std;

int getPid()
{
    string pidPath = Lib::getPath("", PATH_PID);
    ifstream pf(pidPath.c_str());
    char pc[20];
    pf.getline(pc, 20);
    pf.close();
    return atoi(pc);
}

int main(int argc, char const *argv[])
{

    if (argc == 1) {
        cout << "请输入命令代码，0:停止服务|1:启动服务|2:FOK买入" << endl;
        exit(0);
    }

    parseIniFile("config.ini");
    int          traderSrvPort = getOptionToInt("trader_srv_port");
    const char * traderSrvIp   = getOptionToChar("trader_srv_ip");

    int          kLineSrvPort = getOptionToInt("k_line_srv_port");
    const char * kLineSrvIp   = getOptionToChar("k_line_srv_ip");

    string cmd = string(argv[1]);
    if (cmd.compare(CMD_MSG_SHUTDOWN) == 0) {
        // stop market
        int pid = getPid();
        kill(pid, 30);

        // stop trader
        int tcfd = getCSocket(traderSrvIp, traderSrvPort);
        sendMsg(tcfd, CMD_MSG_SHUTDOWN);
        close(tcfd);

        // stop kline
        int kcfd = getCSocket(kLineSrvIp, kLineSrvPort);
        sendMsg(kcfd, CMD_MSG_SHUTDOWN);
        close(kcfd);

    } else if (cmd.compare(CMD_MSG_START) == 0) {
        system("./tradeSrv &");
        system("./kLineSrv &");
        sleep(1);
        system("./marketSrv &");
    }
    return 0;
}
