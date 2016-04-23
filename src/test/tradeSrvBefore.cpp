#include "../global.h"

int main(int argc, char const *argv[])
{
    char p[10];
    strcpy(p, argv[1]);
    double price = atof(p);

    char b[2];
    strcpy(b, argv[2]);
    bool isBuy = strcmp(b, "1") == 0 ? true : false;

    char oc[2];
    strcpy(oc, argv[3]);
    int isOpen= strcmp(oc, "1") == 0 ? true : false;

    char t[10];
    strcpy(t, argv[4]);
    int type = atoi(t); 
    // #define MSG_ORDER 12
    // #define MSG_ORDER_CANCEL 13

    parseIniFile("../etc/config.ini");
    int tradeSrvID = getOptionToInt("trade_service_id");
    QClient clt(tradeSrvID, sizeof(MSG_TO_TRADE));

    MSG_TO_TRADE msg = {0};
    msg.msgType = type;
    msg.price = price;
    msg.isBuy = isBuy;
    msg.total = 1;
    msg.isOpen = isOpen;
    msg.orderID = 1;
    clt.send((void *)&msg);

    return 0;
}
