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

    service = new TradeStrategy(tradeSrvID, logPath);

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
    cout << "MSG:" << msgType << endl;
    if (msgType == MSG_SHUTDOWN) {
        if (service) delete service;
        return false;
    }

    // 下单操作
    if (msgType == MSG_TRADE_BUYOPEN) {
        service->tradeAction(TRADE_ACTION_BUYOPEN, ((MSG_TO_TRADE_STRATEGY*)data)->price, 1);
    }
    if (msgType == MSG_TRADE_SELLOPEN) {
        service->tradeAction(TRADE_ACTION_SELLOPEN, ((MSG_TO_TRADE_STRATEGY*)data)->price, 1);
    }
    if (msgType == MSG_TRADE_SELLCLOSE) {
        service->tradeAction(TRADE_ACTION_SELLCLOSE, ((MSG_TO_TRADE_STRATEGY*)data)->price, 1);
    }
    if (msgType == MSG_TRADE_BUYCLOSE) {
        service->tradeAction(TRADE_ACTION_BUYCLOSE, ((MSG_TO_TRADE_STRATEGY*)data)->price, 1);
    }

    // 下单回馈
    if (msgType == MSG_TRADE_BACK_TRADED) {
        service->onTradeMsgBack(true);
    }
    if (msgType == MSG_TRADE_BACK_CANCELED) {
        service->onTradeMsgBack(false);
    }
    return true;
}

