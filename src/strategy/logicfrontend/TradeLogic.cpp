#include "TradeLogic.h"

TradeLogic::TradeLogic(int peroid, double threshold,
    int serviceID, string logPath, int db,
    string stopTradeTime, string instrumnetID)
{
    _instrumnetID = instrumnetID;

    _peroid = peroid;
    _threshold = threshold;

    _logPath = logPath;

    // 初始化模型参数
    _pUp2Up = _pUp2Down = _pDown2Up = _pDown2Down = 0;
    _countUp2Up = _countUp2Down = _countDown2Up = _countDown2Down = 0;

    // 初始化停止交易时间
    std::vector<string> times = Lib::split(stopTradeTime, "/");
    std::vector<string> hm;
    int i;
    for (i = 0; i < times.size(); ++i)
    {
        TRADE_HM tmp = {0};
        hm = Lib::split(times[i], ":");
        tmp.hour = Lib::stoi(hm[0]);
        tmp.min = Lib::stoi(hm[1]);
        _timeHM.push_back(tmp);
    }

    _store = new Redis("127.0.0.1", 6379, db);
    _tradeStrategySrvClient = new QClient(serviceID, sizeof(MSG_TO_TRADE_STRATEGY));

}

TradeLogic::~TradeLogic()
{
    // delete _store;
    // delete _tradeStrategySrvClient;
    cout << "~TradeLogicSrv" << endl;
}

void TradeLogic::init()
{
    string tickData = _store->get("MARKOV_HISTORY_KLINE_TICK_" + _instrumnetID);
    std::vector<string> ticks = Lib::split(tickData, "_");
    TickData tick = {0};
    for (int i = 0; i < ticks.size(); ++i)
    {
        tick.price = Lib::stod(ticks[i]);
        _tick(tick);
    }
}

void TradeLogic::_tick(TickData tick)
{
    // 如果相邻tick没变化，则忽略
    if (_tickGroup.size() > 0) {
        TickData last = _tickGroup.front();
        if (last.price == tick.price) return;
    }

    // 保存tick
    _tickGroup.push_front(tick);
    while (_tickGroup.size() >= 4) {
        _tickGroup.pop_back();
    }

    // tick足够三个，计算一组转换
    if (_tickGroup.size() < 3) return;
    TickData t[3];
    list<TickData>::iterator i;
    int j = 2;
    for (i = _tickGroup.begin(); i != _tickGroup.end(); i++) {
        t[j--] = *i;
    }

    if (t[0].price > t[1].price) {
        if (t[1].price > t[2].price) {
            _countDown2Down++;
            _transTypeList.push_front(TRANS_TYPE_DOWN2DOWN);
        } else {
            _countDown2Up++;
            _transTypeList.push_front(TRANS_TYPE_DOWN2UP);
        }
    } else {
        if (t[1].price > t[2].price) {
            _countUp2Down++;
            _transTypeList.push_front(TRANS_TYPE_UP2DOWN);
        } else {
            _countUp2Up++;
            _transTypeList.push_front(TRANS_TYPE_UP2UP);
        }
    }

    // 检查转换列表是否够用，够用删除相应的记录
    while (_transTypeList.size() > _peroid) {
        int type = _transTypeList.back();
        _transTypeList.pop_back();
        switch (type) {
            case TRANS_TYPE_DOWN2DOWN:
                _countDown2Down--;
                break;
            case TRANS_TYPE_UP2DOWN:
                _countUp2Down--;
                break;
            case TRANS_TYPE_DOWN2UP:
                _countDown2Up--;
                break;
            case TRANS_TYPE_UP2UP:
                _countUp2Up--;
                break;
            default:
                break;
        }
    }
}

void TradeLogic::_calculateUp()
{
    if (_countUp2Down + _countUp2Up > 0) {
        _pUp2Up = (double)_countUp2Up / ((double)_countUp2Up + (double)_countUp2Down);
        _pUp2Down = 1 - _pUp2Up;
    }
    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeLogicSrv[calculateUp]";
    info << "|iID|" << _instrumnetID;
    info << "|pUp2Up|" << _pUp2Up;
    info << "|pUp2Down|" << _pUp2Down;
    info << "|UU_UD_DU_DD|" << _countUp2Up << "," << _countUp2Down << "," << _countDown2Up << "," << _countDown2Down;
    info << "|kIndex|" << _kIndex << endl;
    info.close();
}

void TradeLogic::_calculateDown()
{
    if (_countDown2Up + _countDown2Down > 0) {
        _pDown2Up = (double)_countDown2Up / ((double)_countDown2Up + (double)_countDown2Down);
        _pDown2Down = 1 - _pDown2Up;
    }
    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeLogicSrv[calculateDown]";
    info << "|iID|" << _instrumnetID;
    info << "|pDown2Up|" << _pDown2Up;
    info << "|pDown2Down|" << _pDown2Down;
    info << "|UU_UD_DU_DD|" << _countUp2Up << "," << _countUp2Down << "," << _countDown2Up << "," << _countDown2Down;
    info << "|kIndex|" << _kIndex;
    info << endl;
    info.close();
}

void TradeLogic::onKLineClose(KLineBlock block, TickData tick)
{
    _tick(tick);
    if (_transTypeList.size() < _peroid) {
        return; // 计算转义概率条件不足，不做操作
    }

    _kIndex = block.getIndex();
    list<TickData>::iterator i = _tickGroup.begin();
    i++;
    TickData last = *i;
    bool isUp = true;
    if (last.price > tick.price) isUp = false;

    if (isUp) {
        _calculateUp();
    } else {
        _calculateDown();
    }

    int status = _getStatus();

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeLogicSrv[onKLineClose]";
    info << "|status|" << status;
    info << endl;
    info.close();

    switch (status) {

        case TRADE_STATUS_NOTHING: // 空仓，判断是否开仓

            if (isUp) {
                if (_pUp2Up > _threshold) { // 买开
                    _sendMsg(MSG_TRADE_BUYOPEN, tick.price + 10);
                }
                if (_pUp2Down > _threshold) { // 卖开
                    _sendMsg(MSG_TRADE_SELLOPEN, tick.price - 10);
                }
            } else {
                if (_pDown2Down > _threshold) { // 卖开
                    _sendMsg(MSG_TRADE_SELLOPEN, tick.price - 10);
                }
                if (_pDown2Up > _threshold) { // 买开
                    _sendMsg(MSG_TRADE_BUYOPEN, tick.price + 10);
                }
            }
            break;

        case TRADE_STATUS_BUYOPENED:

            if (isUp) {
                if (_pUp2Up <= _threshold ) { // 不满足买开，平仓
                    _sendMsg(MSG_TRADE_SELLCLOSE, tick.price - 10);
                }

            } else {

                if (_pDown2Up <= _threshold) { // 不满足买开，平
                    _sendMsg(MSG_TRADE_SELLCLOSE, tick.price - 10);
                }
            }

            break;

        case TRADE_STATUS_SELLOPENED:

            if (isUp) {
                if (_pUp2Down <= _threshold) { // 卖开
                    _sendMsg(MSG_TRADE_BUYCLOSE, tick.price + 10);
                }
            } else {
                if (_pDown2Down <= _threshold) { // 卖开
                    _sendMsg(MSG_TRADE_BUYCLOSE, tick.price + 10);
                }
            }
            break;

        // 简化模型非稳定状态放弃处理
        case TRADE_STATUS_BUYOPENING:
        case TRADE_STATUS_SELLOPENING:
        case TRADE_STATUS_SELLCLOSING:
        case TRADE_STATUS_BUYCLOSING:
        default:
            break;
    }
}

int TradeLogic::_getStatus()
{
    string status = _store->get("TRADE_STATUS_" + _instrumnetID);
    return Lib::stoi(status);
}

void TradeLogic::_sendMsg(int msgType, double price, int hasNext)
{
    string now = Lib::getDate("%H:%M");
    std::vector<string> nowHM = Lib::split(now, ":");
    for (int i = 0; i < _timeHM.size(); ++i)
    {
        if (_timeHM[i].hour == Lib::stoi(nowHM[0]) && Lib::stoi(nowHM[1]) > _timeHM[i].min) {
            return;
        }
    }

    MSG_TO_TRADE_STRATEGY msg = {0};
    msg.msgType = msgType;
    msg.price = price;
    msg.kIndex = _kIndex;
    msg.total = 1;
    strcpy(msg.instrumnetID, Lib::stoc(_instrumnetID));
    _tradeStrategySrvClient->send((void *)&msg);

    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeLogicSrv[sendMsg]";
    info << "|iID|" << _instrumnetID;
    info << "|action|" << msgType;
    info << "|price|" << price;
    info << "|kIndex|" << _kIndex << endl;
    info.close();
}

