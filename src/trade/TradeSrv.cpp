#include "TradeSrv.h"
#include "TraderSpi.h"

TradeSrv::TradeSrv(string brokerID, string userID, string password,
    string tradeFront, string instrumnetIDs, string flowPath, string logPath, int serviceID, int db)
{
    _brokerID = brokerID;
    _userID = userID;
    _password = password;
    _tradeFront = tradeFront;
    _flowPath = flowPath;
    _logPath = logPath;

    std::vector<string> iIDs = Lib::split(instrumnetIDs, "/");
    for (int i = 0; i < iIDs.size(); ++i)
    {
        _ydPostion[iIDs[i]] = 0;
    }

    _store = new Redis("127.0.0.1", 6379, db);
    _tradeStrategySrvClient = new QClient(serviceID, sizeof(MSG_TO_TRADE_STRATEGY));

}

TradeSrv::~TradeSrv()
{
    // delete _store;
    // delete _tradeStrategySrvClient;
    if (_tradeApi) {
        _tradeApi->Release();
        _tradeApi = NULL;
    }
    if (_traderSpi) {
        delete _traderSpi;
    }
    cout << "~TradeSrv" << endl;
}

// void TradeSrv::setStatus(int status)
// {
//     _store->set("TRADE_STATUS", Lib::itos(status));
// }

void TradeSrv::init()
{
    // 初始化交易接口
    _tradeApi = CThostFtdcTraderApi::CreateFtdcTraderApi(Lib::stoc(_flowPath));
    _traderSpi = new TraderSpi(this, _logPath); // 初始化回调实例
    _tradeApi->RegisterSpi(_traderSpi);
    // _tradeApi->SubscribePrivateTopic(THOST_TERT_QUICK);
    // _tradeApi->SubscribePublicTopic(THOST_TERT_QUICK);
    _tradeApi->SubscribePrivateTopic(THOST_TERT_RESUME);
    _tradeApi->SubscribePublicTopic(THOST_TERT_RESUME);

    _tradeApi->RegisterFront(Lib::stoc(_tradeFront));
    _tradeApi->Init();
}

void TradeSrv::confirm()
{
    CThostFtdcSettlementInfoConfirmField req = {0};
    strcpy(req.BrokerID, Lib::stoc(_brokerID));
    strcpy(req.InvestorID, Lib::stoc(_userID));
    string date = Lib::getDate("%Y%m%d");
    strcpy(req.ConfirmDate, date.c_str());
    string time = Lib::getDate("%H:%M:%S");
    strcpy(req.ConfirmTime, time.c_str());

    int res = _tradeApi->ReqSettlementInfoConfirm(&req, 0);
    Lib::sysReqLog(_logPath, "TradeSrv[auth]", res);
}

void TradeSrv::login()
{
    CThostFtdcReqUserLoginField req = {0};

    strcpy(req.BrokerID, Lib::stoc(_brokerID));
    strcpy(req.UserID, Lib::stoc(_userID));
    strcpy(req.Password, Lib::stoc(_password));

    int res = _tradeApi->ReqUserLogin(&req, 0);
    Lib::sysReqLog(_logPath, "TradeSrv[login]", res);
}

void TradeSrv::onLogin(CThostFtdcRspUserLoginField * const rsp)
{
    if (!rsp) return;
    _frontID = rsp->FrontID;
    _sessionID = rsp->SessionID;
    _maxOrderRef = atoi(rsp->MaxOrderRef);
}

void TradeSrv::getPosition()
{
    std::map<string, int>::iterator i;
    for (i = _ydPostion.begin(); i != _ydPostion.end(); ++i)
    {
        CThostFtdcQryInvestorPositionField req = {0};

        strcpy(req.BrokerID, Lib::stoc(_brokerID));
        strcpy(req.InvestorID, Lib::stoc(_userID));
        strcpy(req.InstrumentID, Lib::stoc(i->first));

        int res = _tradeApi->ReqQryInvestorPosition(&req, 0);
        Lib::sysReqLog(_logPath, "TradeSrv[getPosition]", res);
    }
}

void TradeSrv::onPositionRtn(CThostFtdcInvestorPositionField * const rsp)
{
    if (!rsp) return;
    string iID = string(rsp->InstrumentID);
    _ydPostion[iID] = rsp->Position - rsp->TodayPosition;
}

void TradeSrv::trade(double price, int total, bool isBuy, bool isOpen, int orderID, string instrumnetID)
{
    _initOrderRef(orderID);
    TThostFtdcOffsetFlagEnType flag = THOST_FTDC_OFEN_Open;
    if (!isOpen) {
        flag = THOST_FTDC_OFEN_CloseToday;
        if (_ydPostion[instrumnetID] > 0) {
            flag = THOST_FTDC_OFEN_Close;
        }
    }
    CThostFtdcInputOrderField order = _createOrder(instrumnetID, isBuy, total, price, flag,
            THOST_FTDC_HFEN_Speculation, THOST_FTDC_OPT_LimitPrice, THOST_FTDC_TC_GFD, THOST_FTDC_VC_AV);

    int res = _tradeApi->ReqOrderInsert(&order, _maxOrderRef);
    Lib::sysReqLog(_logPath, "TradeSrv[trade]", res);

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSrv[trade]";
    info << "|price|" << price;
    info << "|orderID|" << orderID;
    info << "|orderRef|" << _maxOrderRef;
    info << endl;
    info.close();

    // save data
    string time = Lib::getDate("%Y/%m/%d-%H:%M:%S", true);
    string data = "trade_" + Lib::itos(orderID) + "_" +
                  Lib::itos(_frontID) + "_" + Lib::itos(_sessionID) + "_" + Lib::itos(_maxOrderRef) + "_" +
                  Lib::dtos(price) + "_" + Lib::itos((int)isBuy) + "_" + Lib::itos((int)isOpen) + "_" +
                  time + "_" + instrumnetID;
    _store->push("ORDER_LOGS", data);
}

void TradeSrv::onTraded(CThostFtdcTradeField * const rsp)
{
    if (!rsp) return;
    string iID = string(rsp->InstrumentID);
    if (_ydPostion[iID] > 0) _ydPostion[iID]--;

    int orderID = _getOrderID(atoi(rsp->OrderRef));
    if (orderID <= 0) return;

    CThostFtdcOrderField orderInfo = _getOrderInfo(orderID, atoi(rsp->OrderRef));
    if (strcmp(orderInfo.ExchangeID, rsp->ExchangeID) != 0 ||
        strcmp(orderInfo.OrderSysID, rsp->OrderSysID) != 0) // 不是我的订单，我就不处理了
        return;

    MSG_TO_TRADE_STRATEGY msg = {0};
    msg.msgType = MSG_TRADE_BACK_TRADED;
    msg.orderID = orderID;
    _tradeStrategySrvClient->send((void *)&msg);

    // 将完成的order删除
    map<int, CThostFtdcOrderField>::iterator iter = _orderMap[orderID].find(atoi(rsp->OrderRef));
    if(iter != _orderMap[orderID].end()) {
        _orderMap[orderID].erase(iter);//列表移除
    }

    // save data
    string time = Lib::getDate("%Y/%m/%d-%H:%M:%S", true);
    string data = "traded_" + string(rsp->OrderRef) + "_" + Lib::itos(_frontID) + "_" + Lib::itos(_sessionID) + "_" +
                  string(rsp->TradeDate) + "_" + string(rsp->TradeTime) + "_" + time + "_" + Lib::dtos(rsp->Price);
    _store->push("ORDER_LOGS", data);
}

void TradeSrv::onOrderRtn(CThostFtdcOrderField * const rsp)
{
    if (!rsp) return;
    if (rsp->SessionID != _sessionID) return;

    int orderID = _getOrderID(atoi(rsp->OrderRef));
    if (orderID <= 0) return;

    _setOrderInfo(orderID, rsp);

    // save data
    char c;
    string str;
    stringstream stream;
    stream << rsp->OrderStatus;
    str = stream.str();

    string time = Lib::getDate("%Y/%m/%d-%H:%M:%S", true);
    string data = "orderRtn_" + string(rsp->OrderRef) + "_" + Lib::itos(_frontID) + "_" + Lib::itos(_sessionID) + "_" +
                  string(rsp->InsertDate) + "_" + string(rsp->InsertTime) + "_" + time + "_" +
                  str;
    _store->push("ORDER_LOGS", data);

    if (rsp->OrderStatus != THOST_FTDC_OST_Canceled) return;
    // 撤单情况
    MSG_TO_TRADE_STRATEGY msg = {0};
    msg.msgType = MSG_TRADE_BACK_CANCELED;
    msg.orderID = orderID;
    _tradeStrategySrvClient->send((void *)&msg);
}

void TradeSrv::cancel(int orderID)
{
    CThostFtdcInputOrderActionField req = {0};
    CThostFtdcOrderField orderInfo;

    map<int, CThostFtdcOrderField>::iterator it;
    for(it = _orderMap[orderID].begin(); it != _orderMap[orderID].end(); ++it) {

        orderInfo = it->second;
        if (!orderInfo.OrderRef) continue;

        ///经纪公司代码
        strncpy(req.BrokerID, orderInfo.BrokerID,sizeof(TThostFtdcBrokerIDType));
        ///投资者代码
        strncpy(req.InvestorID, orderInfo.InvestorID,sizeof(TThostFtdcInvestorIDType));
        ///报单引用
        strncpy(req.OrderRef, orderInfo.OrderRef,sizeof(TThostFtdcOrderRefType));
        ///前置编号
        req.FrontID = orderInfo.FrontID;
        ///会话编号
        req.SessionID = orderInfo.SessionID;
        ///交易所代码
        strncpy(req.ExchangeID, orderInfo.ExchangeID, sizeof(TThostFtdcExchangeIDType));
        ///报单编号
        strncpy(req.OrderSysID, orderInfo.OrderSysID, sizeof(TThostFtdcOrderSysIDType));
        ///操作标志
        req.ActionFlag = THOST_FTDC_AF_Delete;
        ///合约代码
        strncpy(req.InstrumentID, orderInfo.InstrumentID, sizeof(TThostFtdcInstrumentIDType));

        int res = _tradeApi->ReqOrderAction(&req, Lib::stoi(orderInfo.OrderRef));
        Lib::sysReqLog(_logPath, "TradeSrv[cancel]", res);

        ofstream info;
        Lib::initInfoLogHandle(_logPath, info);
        info << "TradeSrv[cancel]";
        info << "|orderID|" << orderID;
        info << "|orderRef|" << orderInfo.OrderRef;
        info << "|OrderSysID|" << orderInfo.OrderSysID;
        info << endl;
        info.close();

    }

}

void TradeSrv::onCancel(CThostFtdcInputOrderActionField * const rsp)
{
    if (!rsp) return;
    // nothing to do
}

void TradeSrv::_initOrderRef(int orderID)
{
    _maxOrderRef++;
    _orderRefMap[_maxOrderRef] = orderID;
    CThostFtdcOrderField info = {0};
    _orderMap[orderID][_maxOrderRef] = info;
}

void TradeSrv::_setOrderInfo(int orderID, CThostFtdcOrderField * const pOrderInfo)
{
    _orderMap[orderID][atoi(pOrderInfo->OrderRef)] = *pOrderInfo;
}

CThostFtdcOrderField TradeSrv::_getOrderInfo(int orderID, int orderRef)
{
    return _orderMap[orderID][orderRef];
}

int TradeSrv::_getOrderID(int orderRef)
{
    return _orderRefMap[orderRef];
}

CThostFtdcInputOrderField TradeSrv::_createOrder(string instrumnetID, bool isBuy, int total, double price,
    // double stopPrice,
    TThostFtdcOffsetFlagEnType offsetFlag, // 开平标志
    TThostFtdcHedgeFlagEnType hedgeFlag, // 投机套保标志
    TThostFtdcOrderPriceTypeType priceType, // 报单价格条件
    TThostFtdcTimeConditionType timeCondition, // 有效期类型
    TThostFtdcVolumeConditionType volumeCondition, //成交量类型
    TThostFtdcContingentConditionType contingentCondition// 触发条件
    )
{
    CThostFtdcInputOrderField order = {0};

    strcpy(order.BrokerID, _brokerID.c_str()); ///经纪公司代码
    strcpy(order.InvestorID, _userID.c_str()); ///投资者代码
    strcpy(order.InstrumentID, instrumnetID.c_str()); ///合约代码
    strcpy(order.UserID, _userID.c_str()); ///用户代码
    // strcpy(order.ExchangeID, "SHFE"); ///交易所代码

    order.MinVolume = 1;///最小成交量
    order.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;///强平原因
    order.IsAutoSuspend = 0;///自动挂起标志
    order.UserForceClose = 0;///用户强评标志
    order.IsSwapOrder = 0;///互换单标志

    order.Direction = isBuy ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell; ///买卖方向
    order.VolumeTotalOriginal = total;///数量
    order.LimitPrice = price;///价格
    order.StopPrice = 0;///止损价

    ///组合开平标志
    //THOST_FTDC_OFEN_Open 开仓
    //THOST_FTDC_OFEN_Close 平仓
    //THOST_FTDC_OFEN_ForceClose 强平
    //THOST_FTDC_OFEN_CloseToday 平今
    //THOST_FTDC_OFEN_CloseYesterday 平昨
    //THOST_FTDC_OFEN_ForceOff 强减
    //THOST_FTDC_OFEN_LocalForceClose 本地强平
    order.CombOffsetFlag[0] = offsetFlag;
    if (THOST_FTDC_OFEN_ForceClose == offsetFlag) {
        order.ForceCloseReason = THOST_FTDC_FCC_Other; // 其他
        order.UserForceClose = 1;
    }

    ///组合投机套保标志
    // THOST_FTDC_HFEN_Speculation 投机
    // THOST_FTDC_HFEN_Arbitrage 套利
    // THOST_FTDC_HFEN_Hedge 套保
    order.CombHedgeFlag[0] = hedgeFlag;

    ///报单价格条件
    // THOST_FTDC_OPT_AnyPrice 任意价
    // THOST_FTDC_OPT_LimitPrice 限价
    // THOST_FTDC_OPT_BestPrice 最优价
    // THOST_FTDC_OPT_LastPrice 最新价
    // THOST_FTDC_OPT_LastPricePlusOneTicks 最新价浮动上浮1个ticks
    // THOST_FTDC_OPT_LastPricePlusTwoTicks 最新价浮动上浮2个ticks
    // THOST_FTDC_OPT_LastPricePlusThreeTicks 最新价浮动上浮3个ticks
    // THOST_FTDC_OPT_AskPrice1 卖一价
    // THOST_FTDC_OPT_AskPrice1PlusOneTicks 卖一价浮动上浮1个ticks
    // THOST_FTDC_OPT_AskPrice1PlusTwoTicks 卖一价浮动上浮2个ticks
    // THOST_FTDC_OPT_AskPrice1PlusThreeTicks 卖一价浮动上浮3个ticks
    // THOST_FTDC_OPT_BidPrice1 买一价
    // THOST_FTDC_OPT_BidPrice1PlusOneTicks 买一价浮动上浮1个ticks
    // THOST_FTDC_OPT_BidPrice1PlusTwoTicks 买一价浮动上浮2个ticks
    // THOST_FTDC_OPT_BidPrice1PlusThreeTicks 买一价浮动上浮3个ticks
    // THOST_FTDC_OPT_FiveLevelPrice 五档价
    order.OrderPriceType = priceType;

    ///有效期类型
    // THOST_FTDC_TC_IOC 立即完成，否则撤销
    // THOST_FTDC_TC_GFS 本节有效
    // THOST_FTDC_TC_GFD 当日有效
    // THOST_FTDC_TC_GTD 指定日期前有效
    // THOST_FTDC_TC_GTC 撤销前有效
    // THOST_FTDC_TC_GFA 集合竞价有效
    order.TimeCondition = timeCondition;

    ///成交量类型
    // THOST_FTDC_VC_AV 任何数量
    // THOST_FTDC_VC_MV 最小数量
    // THOST_FTDC_VC_CV 全部数量
    order.VolumeCondition = volumeCondition;

    ///触发条件
    // THOST_FTDC_CC_Immediately 立即
    // THOST_FTDC_CC_Touch 止损
    // THOST_FTDC_CC_TouchProfit 止赢
    // THOST_FTDC_CC_ParkedOrder 预埋单
    // THOST_FTDC_CC_LastPriceGreaterThanStopPrice 最新价大于条件价
    // THOST_FTDC_CC_LastPriceGreaterEqualStopPrice 最新价大于等于条件价
    // THOST_FTDC_CC_LastPriceLesserThanStopPrice 最新价小于条件价
    // THOST_FTDC_CC_LastPriceLesserEqualStopPrice 最新价小于等于条件价
    // THOST_FTDC_CC_AskPriceGreaterThanStopPrice 卖一价大于条件价
    // THOST_FTDC_CC_AskPriceGreaterEqualStopPrice 卖一价大于等于条件价
    // THOST_FTDC_CC_AskPriceLesserThanStopPrice 卖一价小于条件价
    // THOST_FTDC_CC_AskPriceLesserEqualStopPrice 卖一价小于等于条件价
    // THOST_FTDC_CC_BidPriceGreaterThanStopPrice 买一价大于条件价
    // THOST_FTDC_CC_BidPriceGreaterEqualStopPrice 买一价大于等于条件价
    // THOST_FTDC_CC_BidPriceLesserThanStopPrice 买一价小于条件价
    // THOST_FTDC_CC_BidPriceLesserEqualStopPrice 买一价小于等于条件价
    order.ContingentCondition = contingentCondition;

    ///报单引用
    sprintf(order.OrderRef, "%d", _maxOrderRef);

    ///请求编号
    // _reqID++;
    order.RequestID = _maxOrderRef;

    // order.GTDDate = ;///GTD日期
    // order.BusinessUnit = ;///业务单元
    // order.InvestUnitID = ;///投资单元代码
    // order.AccountID = ;///资金账号
    // order.CurrencyID = ;///币种代码
    // order.ClientID = ;///交易编码
    // order.IPAddress = ;///IP地址
    // order.MacAddress = ;///Mac地址

    return order;
}
