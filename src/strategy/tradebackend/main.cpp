#include "TradeStrategy.h"

TradeStrategy * service;
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

    int isDev = getOptionToInt("is_dev");
    int db;
    if (isDev) {
        db = getOptionToInt("rds_db_dev");
    } else {
        db = getOptionToInt("rds_db_online");
    }

    service = new TradeStrategy(tradeSrvID, logPath, db);

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
        if (service) delete service;
        return false;
    }
    MSG_TO_TRADE_STRATEGY msg = *((MSG_TO_TRADE_STRATEGY*)data);
    // cout << "|PRICE|" << msg.price << "|KINDEX|"  << msg.kIndex << endl;
    // 下单操作
    if (msgType == MSG_TRADE_BUYOPEN) {
        service->tradeAction(TRADE_ACTION_BUYOPEN, msg.price, 1, msg.kIndex, msg.hasNext, string(msg.instrumnetID));
    }
    if (msgType == MSG_TRADE_SELLOPEN) {
        service->tradeAction(TRADE_ACTION_SELLOPEN, msg.price, 1, msg.kIndex, msg.hasNext, string(msg.instrumnetID));
    }
    if (msgType == MSG_TRADE_SELLCLOSE) {
        service->tradeAction(TRADE_ACTION_SELLCLOSE, msg.price, 1, msg.kIndex, msg.hasNext, string(msg.instrumnetID));
    }
    if (msgType == MSG_TRADE_BUYCLOSE) {
        service->tradeAction(TRADE_ACTION_BUYCLOSE, msg.price, 1, msg.kIndex, msg.hasNext, string(msg.instrumnetID));
    }

    // if (msgType == MSG_TRADE_CANCEL) {
    //     service->tradeAction(TRADE_ACTION_CANCEL, msg.price, 1, msg.kIndex);
    // }

    // 下单回馈
    if (msgType == MSG_TRADE_BACK_TRADED) {
        service->onSuccess(msg.orderID);
    }
    if (msgType == MSG_TRADE_BACK_CANCELED) {
        service->onCancel(msg.orderID);
    }
    return true;
}

