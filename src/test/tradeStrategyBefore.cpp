#include "../global.h"

int main(int argc, char const *argv[])
{
    char p[10];
    strcpy(p, argv[1]);
    double price = atof(p);

    char k[10];
    strcpy(k, argv[2]);
    int kIndex = atoi(k);

    char t[10];
    strcpy(t, argv[3]);
    int type = atoi(t); 
    // #define MSG_SHUTDOWN 1
    // #define MSG_TRADE_BUYOPEN   5
    // #define MSG_TRADE_SELLOPEN  6
    // #define MSG_TRADE_BUYCLOSE  7
    // #define MSG_TRADE_SELLCLOSE 8
    // #define MSG_TRADE_CANCEL    9

    parseIniFile("../etc/config.ini");
    int sid = getOptionToInt("trade_strategy_service_id");
    QClient clt(sid, sizeof(MSG_TO_TRADE_STRATEGY));

    MSG_TO_TRADE_STRATEGY msg = {0};
    msg.msgType = type;
    msg.price = price;
    msg.kIndex = kIndex;
    cout << msg.price << "|" << price << endl;
    clt.send((void *)&msg);

    return 0;
}
