#include "TradeLogic.h"

TradeLogic * service;

bool action(long int, const void *);

int main(int argc, char const *argv[])
{
    // 初始化参数
    parseIniFile("../etc/config.ini");
    int openMaxKCount        = getOptionToInt("open_max_k_count");
    int openMinKCount        = getOptionToInt("open_min_k_count");
    int openMeanKCount       = getOptionToInt("open_mean_k_count");
    int kRang                = getOptionToInt("k_range");
    int closeSellKRangeCount = getOptionToInt("close_sell_k_range_count");
    int closeBuyKRangeCount  = getOptionToInt("close_buy_k_range_count");

    int tradeLogicSrvID    = getOptionToInt("trade_logic_service_id");
    int tradeStrategySrvID = getOptionToInt("trade_strategy_service_id");

    int isHistoryBack = getOptionToInt("history_back");

    string logPath = getOptionToString("log_path");

    int isDev = getOptionToInt("is_dev");
    int db;
    if (isDev) {
        db = getOptionToInt("rds_db_dev");
    } else {
        db = getOptionToInt("rds_db_online");
    }

    service = new TradeLogic(openMaxKCount, openMinKCount, openMeanKCount, kRang,
        closeSellKRangeCount, closeBuyKRangeCount, tradeStrategySrvID, logPath, isHistoryBack, db);
    service->init();

    // 服务化
    QService Qsrv(tradeLogicSrvID, sizeof(MSG_TO_TRADE_LOGIC));
    Qsrv.setAction(action);
    cout << "TradeLogic service start success!" << endl;
    Qsrv.run();
    cout << "TradeLogic service stop success!" << endl;
    return 0;
}

bool action(long int msgType, const void * data)
{
    // cout << "MSG:" << msgType << endl;
    if (msgType == MSG_SHUTDOWN) {
        if (service) delete service;
        return false;
    }
    if (msgType == MSG_KLINE_OPEN) {
        service->onKLineOpen();
    }
    if (msgType == MSG_KLINE_CLOSE) {
        KLineBlock block = KLineBlock::makeViaData(((MSG_TO_TRADE_LOGIC*)data)->block);
        block.show();
        service->onKLineClose(block);
    }
    return true;
}

