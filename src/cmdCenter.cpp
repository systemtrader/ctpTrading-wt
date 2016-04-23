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

        // // 关闭市场数据源
        // int pid = getPid(getOptionToString("pid_path"));
        // kill(pid, 30);

        // // stop kLineSrv
        // QClient kLineClt(kLineSrvID, sizeof(MSG_TO_KLINE));
        // MSG_TO_KLINE msgkl = {0};
        // msgkl.msgType = MSG_SHUTDOWN;
        // kLineClt.send((void *)&msgkl);

        // // stop tradeLogicSrv
        // QClient tradeLogicClt(tradeLogicSrvID, sizeof(MSG_TO_TRADE_LOGIC));
        // MSG_TO_TRADE_LOGIC msgtl = {0};
        // msgtl.msgType = MSG_SHUTDOWN;
        // tradeLogicClt.send((void *)&msgtl);

        // // stop tradeStrategySrv
        // QClient tradeStrategyClt(tradeStrategySrvID, sizeof(MSG_TO_TRADE_STRATEGY));
        // MSG_TO_TRADE_STRATEGY msgts = {0};
        // msgts.msgType = MSG_SHUTDOWN;
        // tradeStrategyClt.send((void *)&msgts);

        // stop tradeSrv
        QClient tradeClt(tradeSrvID, sizeof(MSG_TO_TRADE));
        MSG_TO_TRADE msgt = {0};
        msgt.msgType = MSG_SHUTDOWN;
        tradeClt.send((void *)&msgt);

    } else if (cmd == 1) {
        // php 相关模块启动，负责初始化redis，队列消费者请手动启动
        // system("php ./readHistoryKLine.php");
        // sleep(1);

        // // 启动各服务模块
        system("./tradeSrv &");
        // system("./tradeStrategySrv &");
        // system("./tradeLogicSrv &");
        // system("./kLineSrv &");
        // sleep(1);

        // 启动数据源
        // system("./marketSrv &");
    }
    return 0;
}
