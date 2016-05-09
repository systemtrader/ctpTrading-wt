#include "KLineSrv.h"
#include <map>

std::map<string, KLineSrv* > services;

bool action(long int, const void *);

int main(int argc, char const *argv[])
{
    // 初始化参数
    parseIniFile("../etc/config.ini");
    int kRange          = getOptionToInt("k_range");
    int kLineSrvID      = getOptionToInt("k_line_service_id");
    int tradeLogicSrvID = getOptionToInt("trade_logic_service_id");
    string logPath      = getOptionToString("log_path");

    int isDev = getOptionToInt("is_dev");
    int db;
    if (isDev) {
        db = getOptionToInt("rds_db_dev");
    } else {
        db = getOptionToInt("rds_db_online");
    }

    string iIDs = getOptionToString("instrumnet_id");
    std::vector<string> instrumnetIDs = Lib::split(iIDs, "/");

    for (int i = 0; i < instrumnetIDs.size(); ++i)
    {
        KLineSrv * tmp = new KLineSrv(kRange, tradeLogicSrvID, logPath, db, instrumnetIDs[i]);
        services[instrumnetIDs[i]] = tmp;
    }

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
    // cout << "MSG:" << msgType << endl;
    if (msgType == MSG_SHUTDOWN) {
        map<string, KLineSrv*>::iterator it;
        for(it = services.begin(); it != services.end(); ++it) {
            delete it->second;
        }
        return false;
    }
    if (msgType == MSG_TICK) {
        TickData tick = ((MSG_TO_KLINE*)data)->tick;
        services[string(tick.instrumnetID)]->onTickCome(tick);
    }
    return true;
}

