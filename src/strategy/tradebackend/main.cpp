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
    int kLineSrvID  = getOptionToInt("k_line_service_id");
    service = new TradeStrategy(tradeSrvID, logPath, db, kLineSrvID);

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

    // if (msgType == MSG_TRADE_CANCEL) {
    //     service->tradeAction(TRADE_ACTION_CANCEL, msg.price, 1, msg.kIndex);
    // }

    // 下单回馈
    if (msgType == MSG_TRADE_BACK_TRADED) {
        service->onSuccess(msg.orderID);
        return true;
    }
    if (msgType == MSG_TRADE_BACK_CANCELED) {
        service->onCancel(msg.orderID);
        return true;
    }
    if (msgType == MSG_TRADE_BACK_CANCELEDERR) {
        service->onCancelErr(msg.orderID);
        return true;
    }
    // 下单操作
    service->accessAction(msg);
    return true;
}

