#include "KLineSrv.h"

KLineSrv * service;

bool action(long int, const void *);

int main(int argc, char const *argv[])
{
    // 初始化参数
    parseIniFile("../etc/config.ini");
    int kRange          = getOptionToInt("k_range");
    int kLineSrvID      = getOptionToInt("k_line_service_id");
    int tradeLogicSrvID = getOptionToInt("trade_logic_service_id");
    string logPath      = getOptionToString("log_path");

    service = new KLineSrv(kRange, tradeLogicSrvID, logPath);

    // 服务化
    QService Qsrv(kLineSrvID, sizeof(MSG_TO_KLINE));
    Qsrv.setAction(action);
    cout << "KLineSrv start success!" << endl;
    Qsrv.run();
    cout << "KLineSrv stop success!" << endl;

    return 0;
}

bool action(long int msgType, const void * data)
{
    cout << "MSG:" << msgType << endl;
    if (msgType == MSG_SHUTDOWN) {
        if (service) delete service;
        return false;
    }
    if (msgType == MSG_TICK) {
        service->onTickCome(((MSG_TO_KLINE*)data)->tick);
    }
    return true;
}

