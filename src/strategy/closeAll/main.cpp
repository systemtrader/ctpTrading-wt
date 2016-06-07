#include "../../global.h"
#include "../global.h"
#include "../../libs/Redis.h"

#define OUU 0
#define OUD 1
#define ODU 2
#define ODD 3
#define CU 4
#define CD 5


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
    info << "CloseAll[sendMsg]";
    info << "|iID|" << instrumnetID;
    info << "|action|" << msgType;
    info << "|price|" << price;
    info << "|kIndex|" << -1 << endl;
    info.close();
}

void sendRollBack(int forecastID, string iID)
{
    //log
    ofstream info;
    Lib::initInfoLogHandle(logPath, info);
    info << "CloseAll[sendRollBack]";
    info << "|iID|" << iID;
    info << "|forecastID|" << forecastID;
    info << endl;
    info.close();

    MSG_TO_TRADE_STRATEGY msg = {0};
    msg.msgType = MSG_TRADE_ROLLBACK;
    msg.forecastID = forecastID;
    msg.isMain = true;
    msg.isForecast = false;
    strcpy(msg.instrumnetID, Lib::stoc(iID));
    tradeStrategySrvClient->send((void *)&msg);

}

void setRollbackID(int type, int id, string iID)
{
    switch (type) {
        case OUU:
            store->set("OUU_" + iID, Lib::itos(id));
            break;
        case OUD:
            store->set("OUD_" + iID, Lib::itos(id));
            break;
        case ODU:
            store->set("ODU_" + iID, Lib::itos(id));
            break;
        case ODD:
            store->set("ODD_" + iID, Lib::itos(id));
            break;
        case CU:
            store->set("CU_" + iID, Lib::itos(id));
            break;
        case CD:
            store->set("CD_" + iID, Lib::itos(id));
            break;
        default:
            break;
    }
}

int getRollbackID(int type, string instrumnetID)
{
    string res;
    switch (type) {
        case OUU:
            res = store->get("OUU_" + instrumnetID);
            break;
        case OUD:
            res = store->get("OUD_" + instrumnetID);
            break;
        case ODU:
            res = store->get("ODU_" + instrumnetID);
            break;
        case ODD:
            res = store->get("ODD_" + instrumnetID);
            break;
        case CU:
            res = store->get("CU_" + instrumnetID);
            break;
        case CD:
            res = store->get("CD_" + instrumnetID);
            break;
        default:
            break;
    }
    return Lib::stoi(res);
}

void rollback(string iID)
{
    int rollbackOpenUUID = getRollbackID(OUU, iID);
    int rollbackOpenUDID = getRollbackID(OUD, iID);
    int rollbackOpenDUID = getRollbackID(ODU, iID);
    int rollbackOpenDDID = getRollbackID(ODD, iID);
    int rollbackCloseUID = getRollbackID(CU, iID);
    int rollbackCloseDID = getRollbackID(CD, iID);

    if (rollbackOpenUUID > 0) {
        sendRollBack(rollbackOpenUUID, iID);
        setRollbackID(OUU, 0, iID);
    }
    if (rollbackOpenUDID > 0) {
        sendRollBack(rollbackOpenUDID, iID);
        setRollbackID(OUD, 0, iID);
    }
    if (rollbackOpenDUID > 0) {
        sendRollBack(rollbackOpenDUID, iID);
        setRollbackID(ODU, 0, iID);
    }
    if (rollbackOpenDDID > 0) {
        sendRollBack(rollbackOpenDDID, iID);
        setRollbackID(ODD, 0, iID);
    }

    if (rollbackCloseUID > 0) {
        sendRollBack(rollbackCloseUID, iID);
        setRollbackID(CU, 0, iID);
    }
    if (rollbackCloseDID > 0) {
        sendRollBack(rollbackCloseDID, iID);
        setRollbackID(CD, 0, iID);
    }
}

bool check(string iID)
{
    string s = store->get("TRADE_STATUS_" + iID);
    int status = Lib::stoi(s);

    string tickStr = store->get("CURRENT_TICK_" + iID);
    TickData tick = Lib::string2TickData(tickStr);

    rollback(iID);
    switch (status) {

        case TRADE_STATUS_BUYOPENED:

            sendMsg(MSG_TRADE_SELLCLOSE, tick.bidPrice1, iID);
            break;

        case TRADE_STATUS_SELLOPENED:

            sendMsg(MSG_TRADE_BUYCLOSE, tick.askPrice1, iID);
            break;

        case TRADE_STATUS_BUYOPENING:
        case TRADE_STATUS_SELLOPENING:
        case TRADE_STATUS_SELLCLOSING:
        case TRADE_STATUS_BUYCLOSING:
        case TRADE_STATUS_NOTHING:
        default:
            break;
    }
    return true;
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
    for (int i = 0; i < iIDs.size(); ++i)
    {
        check(iIDs[i]);
    }
}

