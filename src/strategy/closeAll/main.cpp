#include "../../global.h"
#include "../global.h"
#include "../../libs/Redis.h"

Redis * store;
QClient * tradeStrategySrvClient;
string logPath;

void sendMsg(int msgType, double price, string instrumnetID)
{
    MSG_TO_TRADE_STRATEGY msg = {0};
    msg.msgType = msgType;
    msg.price = price;
    msg.kIndex = -1;
    msg.total = 1;
    strcpy(msg.instrumnetID, Lib::stoc(instrumnetID));
    tradeStrategySrvClient->send((void *)&msg);

    //log
    ofstream info;
    Lib::initInfoLogHandle(logPath, info);
    info << "TradeLogicSrv[sendMsg]";
    info << "|iID|" << instrumnetID;
    info << "|action|" << msgType;
    info << "|price|" << price;
    info << "|kIndex|" << -1 << endl;
    info.close();
}

bool check(string iID)
{
    string s = store->get("TRADE_STATUS_" + iID);
    int status = Lib::stoi(s);

    string tickStr = store->get("CURRENT_TICK");
    TickData tick = Lib::string2TickData(tickStr);

    switch (status) {

        case TRADE_STATUS_BUYOPENED:

            sendMsg(MSG_TRADE_SELLCLOSE, tick.bidPrice1, iID);

        case TRADE_STATUS_SELLOPENED:

            sendMsg(MSG_TRADE_BUYCLOSE, tick.askPrice1, iID);

        case TRADE_STATUS_NOTHING:
            return true;

        case TRADE_STATUS_BUYOPENING:
        case TRADE_STATUS_SELLOPENING:
        case TRADE_STATUS_SELLCLOSING:
        case TRADE_STATUS_BUYCLOSING:
        default:
            return false;
    }
    return false;
}

int main(int argc, char const *argv[])
{
    parseIniFile("../etc/config.ini");

    int isDev = getOptionToInt("is_dev");
    int db;
    if (isDev) {
        db = getOptionToInt("rds_db_dev");
    } else {
        db = getOptionToInt("rds_db_online");
    }
    store = new Redis("127.0.0.1", 6379, db);

    int tradeStrategySrvID = getOptionToInt("trade_strategy_service_id");
    tradeStrategySrvClient = new QClient(tradeStrategySrvID, sizeof(MSG_TO_TRADE_STRATEGY));

    string strIIDs = getOptionToString("instrumnet_id");
    std::vector<string> iIDs = Lib::split(strIIDs, "/");

    logPath = getOptionToString("log_path");

    bool flg = false;
    while (!flg) {
        for (int i = 0; i < iIDs.size(); ++i)
        {
            flg = true && check(iIDs[i]);
        }
    }
}

