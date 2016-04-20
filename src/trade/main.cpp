#include "TradeSrv.h"

TradeSrv * service;

bool action(long int, const void *);

int main(int argc, char const *argv[])
{
    // 初始化参数
    parseIniFile("../etc/config.ini");
    string flowPath = getOptionToString("flow_path");
    string logPath  = getOptionToString("log_path");
    string bid      = getOptionToString("trade_broker_id");
    string userID   = getOptionToString("trade_user_id");
    string password = getOptionToString("trade_password");
    string tURL     = getOptionToString("trade_front");

    string instrumnetID = getOptionToString("instrumnet_id");

    int tradeSrvID         = getOptionToInt("trade_service_id");
    int tradeStrategySrvID = getOptionToInt("trade_strategy_service_id");

    service = new TradeSrv(bid, userID, password, tURL, instrumnetID,
        flowPath, logPath, tradeStrategySrvID);
    service->init();
    
    // 服务化
    QService TradeSrv(tradeSrvID, sizeof(MSG_TO_TRADE));
    TradeSrv.setAction(action);
    cout << "TradeSrv start success!" << endl;
    TradeSrv.run();
    cout << "TradeSrv stop success!" << endl;

    return 0;
}

bool action(long int msgType, const void * data)
{
    cout << "MSG:" << msgType << endl;
    if (msgType == MSG_SHUTDOWN) {
        if (service) delete service;
        return false;
    }
    // ... TODO
    return true;
}

