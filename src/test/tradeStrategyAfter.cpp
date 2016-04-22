#include "../global.h"

bool action(long int, const void *);

int type;
int k;
QClient * clt;

int main(int argc, char const *argv[])
{
    char t[10];
    strcpy(t, argv[1]);
    type = atoi(t);

    // 初始化参数
    parseIniFile("../etc/config.ini");

    int tradeSrvID         = getOptionToInt("trade_service_id");
    int sid = getOptionToInt("trade_strategy_service_id");
    clt = new QClient(sid, sizeof(MSG_TO_TRADE_STRATEGY));
    // 服务化
    QService Qsrv(tradeSrvID, sizeof(MSG_TO_TRADE));
    Qsrv.setAction(action);
    cout << "TradeSrv start success!" << endl;
    Qsrv.run();
    cout << "TradeSrv stop success!" << endl;

    return 0;
}

bool action(long int msgType, const void * data)
{
    cout << "MSG:" << msgType << endl;
    if (msgType == MSG_SHUTDOWN) {
        return false;
    }
    MSG_TO_TRADE msg = *((MSG_TO_TRADE*)data);

    MSG_TO_TRADE_STRATEGY back = {0};

    if (msgType == MSG_ORDER_CANCEL) {
        back.msgType = MSG_TRADE_BACK_CANCELED;
        back.kIndex = msg.orderID;
        clt->send((void *)&back);
    }

    if (msgType == MSG_ORDER) {
        if (type == 1) { // 立刻返回成功
            back.msgType = MSG_TRADE_BACK_TRADED;
            back.kIndex = msg.orderID;
            clt->send((void *)&back);
        }
        if (type == 2) { // 等待5s再返回
            if (k == 0) {
                sleep(5);
            } else {
                back.msgType = MSG_TRADE_BACK_TRADED;
                back.kIndex = msg.orderID;
                clt->send((void *)&back);
            }
        }
        if (type == 3) { // 等第二个kclose时再响应
            if (k > 0 && k != msg.orderID) {
                back.msgType = MSG_TRADE_BACK_TRADED;
                back.kIndex = msg.orderID;
                clt->send((void *)&back);
            }
        }
        k = msg.orderID;
    }

    return true;
}

