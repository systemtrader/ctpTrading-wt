#include "../iniReader/iniReader.h"
#include "../Libs/Lib.h"
#include "../Libs/Socket.h"
#include "../cmd.h"
#include <string>
#include <iostream>

int main(int argc, char const *argv[])
{
    parseIniFile("config.ini");

    // int          traderSrvPort = getOptionToInt("trader_srv_port");
    // const char * traderSrvIp   = getOptionToChar("trader_srv_ip");
    int          kLineSrvPort  = getOptionToInt("k_line_srv_port");

    int sfd = getSSocket(kLineSrvPort);

    char buff[1024];
    string msgLine, msg;
    vector<string> params;
    int cfd, n;
    cout << "KLineSrv start success!" << endl;
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
        if (msg == CMD_MSG_SHUTDOWN) {
            break;
        }
        if (msg == CMD_MSG_MARKET_DATA) {
            // todo
        }
    }
    close(sfd);
    return 0;
}
