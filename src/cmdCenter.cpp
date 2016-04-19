#include "global.h"
#include <signal.h>

int getPid(string pidPath)
{
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

    parseIniFile("../etc/config.ini");
    int kLineSrvID         = getOptionToInt("k_line_service_id");
    int tradeLogicSrvID    = getOptionToInt("trade_logic_service_id");
    int tradeStrategySrvID = getOptionToInt("trade_strategy_service_id");
    int tradeSrvID         = getOptionToInt("trade_service_id");

    int cmd = atoi(argv[1]);
    if (cmd == 0) {

        // stop market
        int pid = getPid(getOptionToString("pid_path"));
        kill(pid, 30);

        // stop kline
        QClient klineClient(kLineSrvID, sizeof(MSG_TO_KLINE));
        MSG_TO_KLINE msgkl = {0};
        msgkl.msgType = MSG_SHUTDOWN;
        klineClient.send((void *)&msgkl);

        // stop trade logic
        QClient tradeLogicClient(tradeLogicSrvID, sizeof(MSG_TO_TRADE_LOGIC));
        MSG_TO_TRADE_LOGIC msgtl = {0};
        msgtl.msgType = MSG_SHUTDOWN;
        tradeLogicClient.send((void *)&msgtl);



    } else if (cmd == 1) {
        // system("./tradeSrv &");
        system("./kLineSrv &");
        system("./tradeLogicFrontend &");
        sleep(1);
        // 消费队列
        // system("php ../store/tick.php &");
        // system("php ../store/kLine.php &");
        system("./marketSrv &");
    }
    return 0;
}
