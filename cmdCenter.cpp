#include "cmd.h"
#include "iniReader/iniReader.h"
#include "lib.h"
#include "socket.h"
#include <string>
#include <iostream>
#include <signal.h>
#include <fstream>

using namespace std;

int getPid()
{
    string pidPath = Lib::getPath("", PATH_PID);
    cout << pidPath << endl;
    ifstream pf(pidPath.c_str());
    char pc[20];
    pf.getline(pc, 20);
    pf.close();
    return atoi(pc);
}

int main(int argc, char const *argv[])
{
    if (argc == 1) {
        cout << "请输入命令代码，0:停止服务|1:启动服务" << endl;
        exit(0);
    }

    parseIniFile("config.ini");
    int          traderSrvPort = getOptionToInt("trader_srv_port");
    const char * traderSrvIp   = getOptionToChar("trader_srv_ip");

    string cmd = string(argv[1]);
    if (cmd.compare(CMD_MSG_SHUTDOWN) == 0) {
        // stop market
        int pid = getPid();
        kill(pid, 30);
        cout << pid << endl;
        // stop trader
        int cfd = getCSocket(traderSrvIp, traderSrvPort);
        sendMsg(cfd, CMD_MSG_SHUTDOWN);
        close(cfd);
    } else if (cmd.compare(CMD_MSG_START) == 0) {
        system("./marketSrv &");
        system("./tradeSrv &");
    }
    return 0;
}
