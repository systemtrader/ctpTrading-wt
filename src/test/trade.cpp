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
    int openclose = strcmp(oc, "1") == 0 ? true : false;

    parseIniFile("../etc/config.ini");
    int tradeSrvID = getOptionToInt("trade_service_id");
    QClient clt(tradeSrvID, sizeof(MSG_TO_TRADE));

    MSG_TO_TRADE msg = {0};
    msg.msgType = MSG_ORDER;
    msg.price = price;
    msg.isBuy = isBuy;
    msg.total = 1;
    msg.isOpen = openclose;
    msg.isCancel = false;
    clt.send((void *)&msg);

    return 0;
}
