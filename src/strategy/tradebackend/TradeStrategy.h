#ifndef TRADE_STRATEGY_H
#define TRADE_STRATEGY_H

#include "../../global.h"
#include "../global.h"
#include "../../libs/Redis.h"
#include <sys/time.h>
#include <signal.h>
#include <cstring>
#include <map>
#include <list>

class TradeStrategy
{
private:

    Redis * _store;
    QClient * _tradeSrvClient;
    QClient * _klineClient;
    string _logPath;
    int _orderID;

    std::vector<MSG_TO_TRADE_STRATEGY> _waitingList; // 等待预测结束的暂存
    // std::map<int, MSG_TO_TRADE_STRATEGY> _waitingCancelList; // 多个平仓，用户暂存平仓信息
    // std::map<int, int> _mainAction; // 每个groupID对应的主要操作，最终还原持仓状态

    // 接到订单发送完成消息，开始处理waitingList中的订单
    // 处理订单的重组，发送，或者继续等待
    void _dealForecast();
    void _dealRealCome();
    void _rollback(int);
    void _open(MSG_TO_TRADE_STRATEGY);
    int _close(MSG_TO_TRADE_STRATEGY);

    std::map<int, int> _group2orderMap; // 保存groupID与orderID关系
    std::map<int, MSG_TO_TRADE_STRATEGY> _orderDetail; // 保存orderID与trade详情的关系

    int _initTrade(MSG_TO_TRADE_STRATEGY); // 初始化交易

    void _removeOrderInfo(int);
    bool _isTrading(int);

    void _zhuijia(int); // 追价
    void _cancel(int); // 撤销

    int _getStatus(string);
    void _setStatus(int, string);
    TickData _getTick(string);
    void _sendMsg(double, int, bool, bool, int);

public:
    TradeStrategy(int, string, int, int);
    ~TradeStrategy();

    void accessAction(MSG_TO_TRADE_STRATEGY);
    void onSuccess(int);
    void onCancel(int);
    void timeout(int);

};



#endif
