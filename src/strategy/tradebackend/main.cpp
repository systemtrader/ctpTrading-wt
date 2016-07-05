#include "TradeStrategy.h"

std::map<string, TradeStrategy* > services;
int timeoutSec;

bool action(long int, const void *);

int main(int argc, char const *argv[])
{
    // 初始化参数
    parseIniFile("../etc/config.ini");
    timeoutSec = getOptionToInt("trade_timeout_sec");

    int tradeStrategySrvID = getOptionToInt("trade_strategy_service_id");
    int tradeSrvID         = getOptionToInt("trade_service_id");
    string logPath         = getOptionToString("log_path");
    int kLineSrvID         = getOptionToInt("k_line_service_id");
    int tradeLogicSrvID    = getOptionToInt("trade_logic_service_id");
    string minRanges    = getOptionToString("min_range");

    int isDev = getOptionToInt("is_dev");
    int db;
    if (isDev) {
        db = getOptionToInt("rds_db_dev");
    } else {
        db = getOptionToInt("rds_db_online");
    }

    std::vector<string> minRs = Lib::split(minRanges, "/");

    string krs = getOptionToString("k_range");
    std::vector<string> kRanges = Lib::split(krs, "/");

    string iIDs = getOptionToString("instrumnet_id");
    std::vector<string> instrumnetIDs = Lib::split(iIDs, "/");
    for (int i = 0; i < instrumnetIDs.size(); ++i)
    {
        services[instrumnetIDs[i]] = new TradeStrategy(tradeSrvID, logPath, db, kLineSrvID, tradeLogicSrvID, Lib::stoi(minRs[i]), Lib::stoi(kRanges[i]));
    }


    // 服务化
    QService Qsrv(tradeStrategySrvID, sizeof(MSG_TO_TRADE_STRATEGY));
    Qsrv.setAction(action);
    cout << "TradeStrategy service start success!" << endl;
    Qsrv.run();
    cout << "TradeStrategy service stop success!" << endl;
    return 0;
}

bool action(long int msgType, const void * data)
{
    // cout << "MSG|" <<  msgType;
    if (msgType == MSG_SHUTDOWN) {
        // cout << endl;
        // if (service) delete service;

        map<string, TradeStrategy*>::iterator it;
        for(it = services.begin(); it != services.end(); ++it) {
            delete it->second;
        }
        return false;
    }
    MSG_TO_TRADE_STRATEGY msg = *((MSG_TO_TRADE_STRATEGY*)data);
    string iID = string(msg.instrumnetID);
    // cout << "|PRICE|" << msg.price << "|KINDEX|"  << msg.kIndex << endl;

    // if (msgType == MSG_TRADE_CANCEL) {
    //     service->tradeAction(TRADE_ACTION_CANCEL, msg.price, 1, msg.kIndex);
    // }

    // 下单回馈
    if (msgType == MSG_TRADE_BACK_TRADED) {
        services[iID]->onSuccess(msg);
        return true;
    }
    if (msgType == MSG_TRADE_BACK_CANCELED) {
        services[iID]->onCancel(msg);
        return true;
    }
    if (msgType == MSG_TRADE_BACK_ERR) {
        services[iID]->onErr(msg.orderID, msg.err);
        return true;
    }
    services[iID]->trade(msg);
    return true;
}

