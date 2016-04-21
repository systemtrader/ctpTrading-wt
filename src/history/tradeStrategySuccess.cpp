/**
 * 回测用，代替tradeStrategy模块，下单即成
 */
#include "../global.h"
#include "../strategy/global.h"
#include "../libs/Redis.h"


Redis store("127.0.0.1", 6379, 1);

bool action(long int, const void *);
int main(int argc, char const *argv[])
{
    parseIniFile("../etc/config.ini");
    int tradeStrategySrvID  = getOptionToInt("trade_strategy_service_id");

    // 服务化
    QService tradeStrategySrv(tradeStrategySrvID, sizeof(MSG_TO_TRADE_STRATEGY));
    tradeStrategySrv.setAction(action);
    tradeStrategySrv.run();

    return 0;
}

bool action(long int msgType, const void * data)
{
    // 下单操作
    if (msgType == MSG_TRADE_BUYOPEN) {
        store.set("TRADE_STATUS", Lib::itos(TRADE_STATUS_BUYOPENED));
    }else if (msgType == MSG_TRADE_SELLOPEN) {
        store.set("TRADE_STATUS", Lib::itos(TRADE_STATUS_SELLOPENED));
    } else {
        store.set("TRADE_STATUS", Lib::itos(TRADE_STATUS_NOTHING));
    }
    return true;
}
