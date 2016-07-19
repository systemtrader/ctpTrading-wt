#include "TradeLogic.h"

#define OUU 0
#define OUD 1
#define ODU 2
#define ODD 3
#define CU 4
#define CD 5


TradeLogic::TradeLogic(int peroid, double thresholdTrend, double thresholdVibrate,
    int serviceID, string logPath, int db,
    string stopTradeTime, string startTradeTime, string instrumnetID, int kRange, int serial, int minRange)
{
    _serial = serial - 1;
    _isLock = false;
    _instrumnetID = instrumnetID;
    _minRange = (double)minRange;

    _peroid = peroid;
    _lineRatio = 2 / ((double)peroid * ((double)peroid + 1));
    _thresholdTrend = thresholdTrend;
    _thresholdVibrate = thresholdVibrate;

    _logPath = logPath;
    _forecastID = 0;
    _rollbackOpenUUID = _rollbackOpenUDID = _rollbackOpenDDID = _rollbackOpenDUID = 0;
    _rollbackCloseDID = _rollbackCloseUID = 0;
    _kRange = kRange;
    _isTradeEnd = false;
    _actionKIndex = 0;

    _firstAction = true;

    // 初始化模型参数
    _pUp2Up = _pUp2Down = _pDown2Up = _pDown2Down = 0;
    // _countUp2Up = _countUp2Down = _countDown2Up = _countDown2Down = 0;

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
        _stopHM.push_back(tmp);
    }

    times = Lib::split(startTradeTime, "/");
    for (i = 0; i < times.size(); ++i)
    {
        TRADE_HM tmp = {0};
        hm = Lib::split(times[i], ":");
        tmp.hour = Lib::stoi(hm[0]);
        tmp.min = Lib::stoi(hm[1]);
        _startHM.push_back(tmp);
    }

    _store = new Redis("127.0.0.1", 6379, db);
    _store->set("TRADE_TAG_" + _instrumnetID, "0");
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

bool TradeLogic::_isTradingTime(TickData tick)
{
    string now = string(tick.time);
    std::vector<string> nowHM = Lib::split(now, ":");
    int i;
    ofstream info;
    for (i = 0; i < _stopHM.size(); ++i)
    {
        if ((_stopHM[i].hour == Lib::stoi(nowHM[0]) && Lib::stoi(nowHM[1]) >= _stopHM[i].min) ||
            Lib::stoi(nowHM[0]) == _stopHM[i].hour + 1)
        {
            //log
            Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
            info << "TradeLogicSrv[stopTrade]";
            info << "|nowH|" << nowHM[0];
            info << "|nowM|" << nowHM[1];
            info << "|stopH|" << _stopHM[i].hour;
            info << "|stopM|" << _stopHM[i].min;
            info << endl;
            info.close();
            _isTradeEnd = true;
            return false;
        }
    }
    for (i = 0; i < _startHM.size(); ++i)
    {
        if ((_startHM[i].hour == Lib::stoi(nowHM[0]) && Lib::stoi(nowHM[1]) < _startHM[i].min) ||
            Lib::stoi(nowHM[0]) == _startHM[i].hour - 1) // 前一小时也不能交易
        {
            //log
            Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
            info << "TradeLogicSrv[startTrade]";
            info << "|nowH|" << nowHM[0];
            info << "|nowM|" << nowHM[1];
            info << "|startH|" << _startHM[i].hour;
            info << "|startM|" << _startHM[i].min;
            info << endl;
            info.close();
            return false;
        }
    }
    // 全品种十点十五休息至十点半 提前30秒休息，错后30秒开始
    if ((Lib::stoi(nowHM[0]) == 10 && Lib::stoi(nowHM[1]) == 14 && Lib::stoi(nowHM[2]) >= 30) ||
        // (Lib::stoi(nowHM[0]) == 10 && Lib::stoi(nowHM[1]) == 30 && Lib::stoi(nowHM[2]) >= 30) ||
        (Lib::stoi(nowHM[0]) == 10 && Lib::stoi(nowHM[1]) >= 15 && Lib::stoi(nowHM[1]) <= 29 ))
    {
        //log
        Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
        info << "TradeLogicSrv[10clock]";
        info << "|nowH|" << nowHM[0];
        info << "|nowM|" << nowHM[1];
        info << "|nowS|" << nowHM[2];
        info << endl;
        info.close();
        return false;
    }
    return true;
}

bool TradeLogic::_isSerial()
{
    if (_serial <= 1) return false;
    if (_transTypeList.size() < _serial) return false;
    list<int>::iterator it = _transTypeList.begin();
    list<int>::iterator pre = it;
    it++;
    bool flg = true;
    int i = 0;
    while (it != _transTypeList.end()) {
        if (i >= _serial - 1) break;
        if (*it != *pre) {
            flg = false;
            break;
        }
        i++;
        it++;
        pre++;
    }
    if (flg) _isLock = false;

    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "TradeLogicSrv[isSerial]";
    info << "|isSerial|" << flg;
    info << endl;
    info.close();
    return flg;
}

void TradeLogic::_tick(TickData tick)
{
    _closeTick = tick;
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
            // _countDown2Down++;
            _transTypeList.push_front(TRANS_TYPE_DOWN2DOWN);
        } else {
            // _countDown2Up++;
            _transTypeList.push_front(TRANS_TYPE_DOWN2UP);
        }
    } else {
        if (t[1].price > t[2].price) {
            // _countUp2Down++;
            _transTypeList.push_front(TRANS_TYPE_UP2DOWN);
        } else {
            // _countUp2Up++;
            _transTypeList.push_front(TRANS_TYPE_UP2UP);
        }
    }

    // 检查转换列表是否够用，够用删除相应的记录
    while (_transTypeList.size() > _peroid) {
        int type = _transTypeList.back();
        _transTypeList.pop_back();
        // switch (type) {
        //     case TRANS_TYPE_DOWN2DOWN:
        //         _countDown2Down--;
        //         break;
        //     case TRANS_TYPE_UP2DOWN:
        //         _countUp2Down--;
        //         break;
        //     case TRANS_TYPE_DOWN2UP:
        //         _countDown2Up--;
        //         break;
        //     case TRANS_TYPE_UP2UP:
        //         _countUp2Up--;
        //         break;
        //     default:
        //         break;
        // }
    }
}

void TradeLogic::_calculateUp(int type)
{
    double u2d = 0;
    double u2u = 0;
    list<int>::reverse_iterator i;
    int j = 0;
    for (i = _transTypeList.rbegin(); i != _transTypeList.rend(); ++i)
    {
        if (type != -1 && i == _transTypeList.rbegin()) continue;
        j++;
        if (*i == TRANS_TYPE_UP2UP) u2u = u2u + _lineRatio * j;
        if (*i == TRANS_TYPE_UP2DOWN) u2d = u2d + _lineRatio * j;
    }
    j++;
    if (type == TRANS_TYPE_UP2UP) {
        u2u = u2u + _lineRatio * j;
    }
    if (type == TRANS_TYPE_UP2DOWN) {
        u2d = u2d + _lineRatio * j;
    }

    if (u2d + u2u > 0) {
        _pUp2Up = (double)u2u / ((double)u2u + (double)u2d);
        _pUp2Down = 1 - _pUp2Up;
    }
    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "TradeLogicSrv[calculateUp]";
    info << "|pUp2Up|" << _pUp2Up;
    info << "|pUp2Down|" << _pUp2Down;
    info << "|UU_UD|" << u2u << "," << u2d;
    info << endl;
    info.close();
}


void TradeLogic::_calculateDown(int type)
{
    double d2u = 0;
    double d2d = 0;
    list<int>::reverse_iterator i;
    int j = 0;
    for (i = _transTypeList.rbegin(); i != _transTypeList.rend(); ++i)
    {
        if (type != -1 && i == _transTypeList.rbegin()) continue;
        j++;
        if (*i == TRANS_TYPE_DOWN2UP) d2u = d2u + _lineRatio * j;
        if (*i == TRANS_TYPE_DOWN2DOWN) d2d = d2d + _lineRatio * j;
    }
    j++;
    if (type == TRANS_TYPE_DOWN2UP) {
        d2u = d2u + _lineRatio * j;
    }
    if (type == TRANS_TYPE_DOWN2DOWN) {
        d2d = d2d + _lineRatio * j;
    }

    if (d2u + d2d > 0) {
        _pDown2Up = (double)d2u / ((double)d2u + (double)d2d);
        _pDown2Down = 1 - _pDown2Up;
    }
    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "TradeLogicSrv[calculateDown]";
    info << "|pDown2Up|" << _pDown2Up;
    info << "|pDown2Down|" << _pDown2Down;
    info << "|DU_DD|" << d2u << "," << d2d;
    info << endl;
    info.close();
}

bool TradeLogic::_isCurrentUp()
{
    list<TickData>::iterator it = _tickGroup.begin();
    TickData last = *it;
    it++;
    TickData before = *it;
    bool isUp = true;
    if (last.price < before.price) isUp = false;
    return isUp;
}

void TradeLogic::_setRollbackID(int type, int id)
{
    switch (type) {
        case OUU:
            _rollbackOpenUUID = id;
            _store->set("OUU_" + _instrumnetID, Lib::itos(id));
            break;
        case OUD:
            _rollbackOpenUDID = id;
            _store->set("OUD_" + _instrumnetID, Lib::itos(id));
            break;
        case ODU:
            _rollbackOpenDUID = id;
            _store->set("ODU_" + _instrumnetID, Lib::itos(id));
            break;
        case ODD:
            _rollbackOpenDDID = id;
            _store->set("ODD_" + _instrumnetID, Lib::itos(id));
            break;
        case CU:
            _rollbackCloseUID = id;
            _store->set("CU_" + _instrumnetID, Lib::itos(id));
            break;
        case CD:
            _rollbackCloseDID = id;
            _store->set("CD_" + _instrumnetID, Lib::itos(id));
            break;
        default:
            break;
    }
}

void TradeLogic::onForecastSuccess(int kIndex)
{
    _setTickSwitch(false);
    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "TradeLogicSrv[onForecastSuccess]";
    info << "|kIndex|" << kIndex;
    info << "|currentKIndex|" << _kIndex;
    info << endl;
    info.close();
    if (kIndex == _kIndex)
        _rollback();
}

bool TradeLogic::_rollback()
{
    bool flg = false;
    if (_rollbackOpenUUID > 0) {
        flg = true;
        _sendRollBack(_rollbackOpenUUID);
    }
    if (_rollbackOpenUDID > 0) {
        flg = true;
        _sendRollBack(_rollbackOpenUDID);
    }
    if (_rollbackOpenDUID > 0) {
        flg = true;
        _sendRollBack(_rollbackOpenDUID);
    }
    if (_rollbackOpenDDID > 0) {
        flg = true;
        _sendRollBack(_rollbackOpenDDID);
    }

    if (_rollbackCloseUID > 0) {
        flg = true;
        _sendRollBack(_rollbackCloseUID);
    }
    if (_rollbackCloseDID > 0) {
        flg = true;
        _sendRollBack(_rollbackCloseDID);
    }
    _sendRollBack(-1);
    return flg;
}

bool TradeLogic::_canRollbackOpen(TickData tick)
{
    bool flg = true;
    if (_forecastOpenPrice1 > 0 && abs(tick.price - _forecastOpenPrice1) < _minRange * 2) {
        flg = false;
    }
    if (_forecastOpenPrice2 > 0 && abs(tick.price - _forecastOpenPrice2) < _minRange * 2) {
        flg = false;
    }
    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "TradeLogicSrv[canRollbackOpen]";
    info << "|forecastOpenPrice1|" << _forecastOpenPrice1;
    info << "|forecastOpenPrice2|" << _forecastOpenPrice2;
    info << "|tickPrice|" << tick.price;
    info << "|minRange|" << _minRange;
    info << "|flg|" << flg;
    info << endl;
    info.close();
    return flg;
}

bool TradeLogic::_canRollbackClose(TickData tick, bool isBuyCloseAction)
{
    bool flg = false;
    if (isBuyCloseAction) {
        if (_forecastClosePrice > 0 && abs(tick.askPrice1 - _forecastClosePrice) > 2 * _minRange) {
            flg = true;
        }
    } else {
        if (_forecastClosePrice > 0 && abs(tick.bidPrice1 - _forecastClosePrice) > 2 * _minRange) {
            flg = true;
        }
    }
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "TradeLogicSrv[canRollbackClose]";
    info << "|forecastClosePrice|" << _forecastClosePrice;
    info << "|isBuyCloseAction|" << isBuyCloseAction;
    info << "|bidPrice1|" << tick.bidPrice1;
    info << "|askPrice1|" << tick.askPrice1;
    info << "|minRange|" << _minRange;
    info << "|flg|" << flg;
    info << endl;
    info.close();
    return flg;
}

void TradeLogic::_forecastNothing(TickData tick)
{
    // 只发一单
    if (_isCurrentUp()) { // 当前是up

        // _calculateDown(_countDown2Up, _countDown2Down);
        _calculateDown(TRANS_TYPE_UP2DOWN);
        if (_pDown2Up > _thresholdVibrate) {
            _forecastID++;
            // _rollbackOpenUDID = _forecastID;
            _setRollbackID(OUD, _forecastID);
            _sendMsg(MSG_TRADE_BUYOPEN, tick.price - _kRange, true, _forecastID, 1);
            _forecastOpenPrice1 = tick.price - _kRange;
        }

        // _calculateUp(_countUp2Down, _countUp2Up + 1);
        _calculateUp(TRANS_TYPE_UP2UP);
        if (_pUp2Down > _thresholdVibrate) {
            _forecastID++;
            // _rollbackOpenUUID = _forecastID;
            _setRollbackID(OUU, _forecastID);
            _sendMsg(MSG_TRADE_SELLOPEN, tick.price + _kRange, true, _forecastID, 2);
            _forecastOpenPrice2 = tick.price + _kRange;
        }

    } else { // 当前是down

        // _calculateDown(_countDown2Up, _countDown2Down + 1);
        _calculateDown(TRANS_TYPE_DOWN2DOWN);
        if (_pDown2Up > _thresholdVibrate) {
            _forecastID++;
            // _rollbackOpenDDID = _forecastID;
            _setRollbackID(ODD, _forecastID);
            _sendMsg(MSG_TRADE_BUYOPEN, tick.price - _kRange, true, _forecastID, 1);
            _forecastOpenPrice1 = tick.price - _kRange;
        }

        // _calculateUp(_countUp2Down, _countUp2Up);
        _calculateUp(TRANS_TYPE_DOWN2UP);
        if (_pUp2Down > _thresholdVibrate) {
            _forecastID++;
            // _rollbackOpenDUID = _forecastID;
            _setRollbackID(ODU, _forecastID);
            _sendMsg(MSG_TRADE_SELLOPEN, tick.price + _kRange, true, _forecastID, 2);
            _forecastOpenPrice2 = tick.price + _kRange;
        }
    }
}

void TradeLogic::_forecastBuyOpened(TickData tick)
{
    if (_isCurrentUp()) { // 当前是up

        // _calculateUp(_countUp2Down, _countUp2Up + 1);
        _calculateUp(TRANS_TYPE_UP2UP);
        if (_pUp2Up <= _thresholdTrend) {
            _forecastID++;
            // _rollbackCloseUID = _forecastID;
            _setRollbackID(CU, _forecastID);
            if (_pUp2Down > _thresholdVibrate) {
                _forecastID++;
                // _rollbackOpenUUID = _forecastID;
                _setRollbackID(OUU, _forecastID);
                _sendMsg(MSG_TRADE_SELLCLOSE, tick.price + _kRange, true, _rollbackCloseUID, 3);
                _sendMsg(MSG_TRADE_SELLOPEN, tick.price + _kRange, true, _rollbackOpenUUID, 1);
                _forecastOpenPrice1 = _forecastClosePrice = tick.price + _kRange;
            } else {
                _sendMsg(MSG_TRADE_SELLCLOSE, tick.price + _kRange, true, _rollbackCloseUID, 1);
                _forecastClosePrice = tick.price + _kRange;
            }
        }

    } else { // 当前是down

        // _calculateUp(_countUp2Down, _countUp2Up);
        _calculateUp(TRANS_TYPE_DOWN2UP);
        if (_pUp2Up <= _thresholdTrend) {
            _forecastID++;
            // _rollbackCloseDID = _forecastID;
            _setRollbackID(CD, _forecastID);
            if (_pUp2Down > _thresholdVibrate) {
                _forecastID++;
                // _rollbackOpenDUID = _forecastID;
                _setRollbackID(ODU, _forecastID);
                _sendMsg(MSG_TRADE_SELLCLOSE, tick.price + _kRange, true, _rollbackCloseDID, 3);
                _sendMsg(MSG_TRADE_SELLOPEN, tick.price + _kRange, true, _rollbackOpenDUID, 1);
                _forecastOpenPrice1 = _forecastClosePrice = tick.price + _kRange;
            } else {
                _sendMsg(MSG_TRADE_SELLCLOSE, tick.price + _kRange, true, _rollbackCloseDID, 1);
                _forecastClosePrice = tick.price + _kRange;
            }
        }

    }
}

void TradeLogic::_forecastSellOpened(TickData tick)
{
    bool isCloseMain = true;
    if (_isCurrentUp()) { // 当前是up

        // _calculateDown(_countDown2Up, _countDown2Down);
        _calculateDown(TRANS_TYPE_UP2DOWN);
        if (_pDown2Down <= _thresholdTrend) {
            _forecastID++;
            // _rollbackCloseUID = _forecastID;
            _setRollbackID(CU, _forecastID);
            if (_pDown2Up > _thresholdVibrate) {
                _forecastID++;
                // _rollbackOpenUDID = _forecastID;
                _setRollbackID(OUD, _forecastID);
                _sendMsg(MSG_TRADE_BUYCLOSE, tick.price - _kRange, true, _rollbackCloseUID, 3);
                _sendMsg(MSG_TRADE_BUYOPEN, tick.price - _kRange, true, _rollbackOpenUDID, 1);
                _forecastOpenPrice1 = _forecastClosePrice = tick.price - _kRange;
            } else {
                _sendMsg(MSG_TRADE_BUYCLOSE, tick.price - _kRange, true, _rollbackCloseUID, 1);
                _forecastClosePrice = tick.price - _kRange;
            }
        }

    } else { // 当前是down

        // _calculateDown(_countDown2Up, _countDown2Down + 1);
        _calculateDown(TRANS_TYPE_UP2DOWN);
        if (_pDown2Down <= _thresholdTrend) {
            _forecastID++;
            // _rollbackCloseDID = _forecastID;
            _setRollbackID(CD, _forecastID);
            if (_pDown2Up > _thresholdVibrate) {
                _forecastID++;
                // _rollbackOpenDDID = _forecastID;
                _setRollbackID(ODD, _forecastID);
                _sendMsg(MSG_TRADE_BUYCLOSE, tick.price - _kRange, true, _rollbackCloseDID, 3);
                _sendMsg(MSG_TRADE_BUYOPEN, tick.price - _kRange, true, _rollbackOpenDDID, 1);
                _forecastOpenPrice1 = _forecastClosePrice = tick.price - _kRange;
            } else {
                _sendMsg(MSG_TRADE_BUYCLOSE, tick.price - _kRange, true, _rollbackCloseDID, 1);
                _forecastClosePrice = tick.price - _kRange;
            }
        }
    }
}

void TradeLogic::_forecast(TickData tick)
{
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "TradeLogicSrv[forecastBegin]";

    if (_transTypeList.size() < _peroid - 1) {
        _isLock = false;
        info << "|peroid|" << _peroid;
        info << endl;
        info.close();
        return; // 计算概率条件不足，不做操作
    }

    int status1 = _getStatus(1);

    info << "|status1|" << status1;
    info << endl;
    info.close();

    // _rollbackOpenUUID = _rollbackOpenUDID = _rollbackOpenDDID = _rollbackOpenDUID = 0;
    // _rollbackCloseDID = _rollbackCloseUID = 0;
    _setRollbackID(OUU, 0);
    _setRollbackID(OUD, 0);
    _setRollbackID(ODD, 0);
    _setRollbackID(ODU, 0);
    _setRollbackID(CU, 0);
    _setRollbackID(CD, 0);
    _setStatus(2, TRADE_STATUS_UNKOWN);
    _setStatus(3, TRADE_STATUS_UNKOWN);
    _forecastOpenPrice1 = _forecastOpenPrice2 = _forecastClosePrice = 0;
    _setTickSwitch(false);
    switch (status1) {
        case TRADE_STATUS_NOTHING:
            if (!_isSerial()) {
                _forecastNothing(tick);
            }
            break;
        case TRADE_STATUS_BUYOPENED:
            _forecastBuyOpened(tick);
            break;
        case TRADE_STATUS_SELLOPENED:
            _forecastSellOpened(tick);
            break;
        default:
            break;
    }
    _isLock = false;
}

void TradeLogic::_realAction(TickData tick)
{
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "TradeLogicSrv[realAction]";

    if (_transTypeList.size() < _peroid - 1) {
        _isLock = false;
        info << "|peroid|" << _peroid;
        info << endl;
        info.close();
        return; // 计算概率条件不足，不做操作
    }

    int status1 = _getStatus(1);

    info << "|status1|" << status1;
    info << endl;
    info.close();

    TickData nowTick = _getTick();

    switch (status1) {
        case TRADE_STATUS_NOTHING:
            if (_isSerial()) break;
            if (_isCurrentUp()) {
                // _calculateUp(_countUp2Down, _countUp2Up);
                _calculateUp();
                if (_pUp2Up > _thresholdTrend) { // 买开
                    _sendMsg(MSG_TRADE_BUYOPEN, tick.price, false, 0, 1, true);
                } else {
                    if (!_firstAction) {
                        _forecast(tick);
                    } else {
                        _firstAction = false;
                        if (_pUp2Down > _thresholdVibrate) {
                            _sendMsg(MSG_TRADE_SELLOPEN, tick.price, false, 0, 1, true);
                        } else {
                            _forecast(tick);
                        }
                    }
                }
            } else {
                // _calculateDown(_countDown2Up, _countDown2Down);
                _calculateDown();
                if (_pDown2Down > _thresholdTrend) { // 卖开
                    _sendMsg(MSG_TRADE_SELLOPEN, tick.price, false, 0, 1, true);
                } else {
                    if (!_firstAction) {
                        _forecast(tick);
                    } else {
                        _firstAction = false;
                        if (_pUp2Down > _thresholdVibrate) {
                            _sendMsg(MSG_TRADE_BUYOPEN, tick.price, false, 0, 1, true);
                        } else {
                            _forecast(tick);
                        }
                    }
                }
            }
            break;

        case TRADE_STATUS_SELLOPENED:

            if (_isCurrentUp()) {
                // _calculateUp(_countUp2Down, _countUp2Up);
                _calculateUp();
                if (_pUp2Down <= _thresholdVibrate) { // 不满足买开，平仓
                    if (_pUp2Up > _thresholdTrend) {
                        _sendMsg(MSG_TRADE_BUYCLOSE, nowTick.bidPrice1, false, 0, 1, true);
                        if (!_isSerial()) _sendMsg(MSG_TRADE_BUYOPEN, tick.price, false, 0, 1, true);
                    } else {
                        _sendMsg(MSG_TRADE_BUYCLOSE, nowTick.bidPrice1, false, 0, 1, true);
                    }
                } else {
                    if (_isSerial()) {
                        _sendMsg(MSG_TRADE_BUYCLOSE, nowTick.bidPrice1, false, 0, 1, true); // 只交易一根K线，无论如果都平仓
                    } else {
                        _forecast(tick);
                    }
                }

            } else {
                // _calculateDown(_countDown2Up, _countDown2Down);
                _calculateDown();
                if (_pDown2Down <= _thresholdTrend) { // 不满足买开，平
                    if (_pDown2Up > _thresholdVibrate) {
                        _sendMsg(MSG_TRADE_BUYCLOSE, nowTick.bidPrice1, false, 0, 1, true);
                        if (!_isSerial()) _sendMsg(MSG_TRADE_BUYOPEN, tick.price, false, 0, 1, true);
                    } else {
                        _sendMsg(MSG_TRADE_BUYCLOSE, nowTick.bidPrice1, false, 0, 1, true);
                    }
                } else {
                    if (_isSerial()){
                        _sendMsg(MSG_TRADE_BUYCLOSE, nowTick.bidPrice1, false, 0, 1, true); // 只交易一根K线，无论如果都平仓
                    } else {
                        _forecast(tick);
                    }
                }
            }
            break;

        case TRADE_STATUS_BUYOPENED:

            if (_isCurrentUp()) {
                // _calculateUp(_countUp2Down, _countUp2Up);
                _calculateUp();
                if (_pUp2Up <= _thresholdTrend ) { // 不满足买开，平仓
                    if (_pUp2Down > _thresholdVibrate) {
                        _sendMsg(MSG_TRADE_SELLCLOSE, nowTick.askPrice1, false, 0, 1, true);
                        if (!_isSerial()) _sendMsg(MSG_TRADE_SELLOPEN, tick.price, false, 0, 1, true);
                    } else {
                        _sendMsg(MSG_TRADE_SELLCLOSE, nowTick.askPrice1, false, 0, 1, true);
                    }
                } else {
                    if (_isSerial()) {
                        _sendMsg(MSG_TRADE_SELLCLOSE, nowTick.askPrice1, false, 0, 1, true);
                    } else {
                        _forecast(tick);
                    }
                }

            } else {
                // _calculateDown(_countDown2Up, _countDown2Down);
                _calculateDown();
                if (_pDown2Up <= _thresholdVibrate) { // 不满足买开，平
                    if (_pDown2Down > _thresholdTrend) {
                        _sendMsg(MSG_TRADE_SELLCLOSE, nowTick.askPrice1, false, 0, 1, true);
                        if (!_isSerial()) _sendMsg(MSG_TRADE_SELLOPEN, tick.price, false, 0, 1, true);
                    } else {
                        _sendMsg(MSG_TRADE_SELLCLOSE, nowTick.askPrice1, false, 0, 1, true);
                    }
                } else {
                    if (_isSerial())
                        _sendMsg(MSG_TRADE_SELLCLOSE, nowTick.askPrice1, false, 0, 1, true);
                    else
                        _forecast(tick);
                }
            }
            break;
    }
}

void TradeLogic::_endClose()
{
    int status1 = _getStatus(1);
    int status2 = _getStatus(2);
    int status3 = _getStatus(3);

    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "TradeLogicSrv[endClose]";
    info << "|status1|" << status1;
    info << "|status2|" << status2;
    info << "|status3|" << status3;
    info << endl;
    info.close();

    TickData tick = _getTick();

    _kIndex = -1;
    if (status1 == TRADE_STATUS_BUYOPENED) {
        _sendMsg(MSG_TRADE_SELLCLOSE, tick.askPrice1, false, 0, 1, true);
    }
    if (status2 == TRADE_STATUS_BUYOPENED) {
        _sendMsg(MSG_TRADE_SELLCLOSE, tick.askPrice1, false, 0, 2, true);
    }
    if (status3 == TRADE_STATUS_BUYOPENED) {
        _sendMsg(MSG_TRADE_SELLCLOSE, tick.askPrice1, false, 0, 3, true);
    }
    if (status1 == TRADE_STATUS_SELLOPENED) {
        _sendMsg(MSG_TRADE_BUYCLOSE, tick.bidPrice1, false, 0, 1, true);
    }
    if (status2 == TRADE_STATUS_SELLOPENED) {
        _sendMsg(MSG_TRADE_BUYCLOSE, tick.bidPrice1, false, 0, 2, true);
    }
    if (status3 == TRADE_STATUS_SELLOPENED) {
        _sendMsg(MSG_TRADE_BUYCLOSE, tick.bidPrice1, false, 0, 3, true);
    }
}

void TradeLogic::onTradeEnd()
{
    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "TradeLogicSrv[onTradeEnd]";
    info << endl;
    info.close();

    _isTradeEnd = true;
    if (!_rollback()) {
        _endClose();
    }

    _setRollbackID(OUU, 0);
    _setRollbackID(OUD, 0);
    _setRollbackID(ODD, 0);
    _setRollbackID(ODU, 0);
    _setRollbackID(CU, 0);
    _setRollbackID(CD, 0);

}

void TradeLogic::onKLineClose(KLineBlock block, TickData tick)
{

    _tick(tick);
    _kIndex = block.getIndex();
    if (!_isTradingTime(tick)) return;

    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "TradeLogicSrv[onKLineClose]";
    info << "|isLock|" << _isLock;

    if (_isLock) {
        info << endl;
        info.close();
        return;
    }

    // 增加中间控制，标志开启后空仓，恢复后正常
    string tagStr = _store->get("TRADE_TAG_" + _instrumnetID);
    int tag = Lib::stoi(tagStr);
    switch (tag) {
        case 0: // 正常
            break;
        case 1: // 返回
            return;
        case 2: // 暂停
            _store->set("TRADE_TAG_" + _instrumnetID, "1");
            onTradeEnd();
            return;
        case 3: // 恢复
            _store->set("TRADE_TAG_" + _instrumnetID, "0");
            _isTradeEnd = false;
        default:
            break;
    }
    _isLock = true;

    int status1 = _getStatus(1);
    int status2 = _getStatus(2);
    int status3 = _getStatus(3);

    info << "|status1|" << status1;
    info << "|status2|" << status2;
    info << "|status3|" << status3;
    info << endl;
    info.close();

    switch (status1) {

        case TRADE_STATUS_NOTHING:

            _statusType = STATUS_TYPE_CLOSE_NO;
            if (_kIndex == _actionKIndex) {
                _statusType = STATUS_TYPE_CLOSE_WAIT_NO;
            }

            if (status2 == TRADE_STATUS_SELLOPENING) {
                _statusType = STATUS_TYPE_CLOSE_NO_SOING;
            }
            break;
        case TRADE_STATUS_SELLOPENED:

            _statusType = STATUS_TYPE_CLOSE_SOED;

            if (_kIndex == _actionKIndex) {
                _statusType = STATUS_TYPE_CLOSE_WAIT_SOED;
            }

            if (status3 == TRADE_STATUS_SELLCLOSING) {
                _statusType = STATUS_TYPE_CLOSE_WAIT_SOED__SCING;
            }

            if (status3 == TRADE_STATUS_NOTHING) {
                _statusType = STATUS_TYPE_CLOSE_WAIT_SOED__NO;
            }
            break;

        case TRADE_STATUS_BUYOPENED:

            _statusType = STATUS_TYPE_CLOSE_BOED;
            if (_kIndex == _actionKIndex) {
                _statusType = STATUS_TYPE_CLOSE_WAIT_BOED;
            }

            if (status2 == TRADE_STATUS_SELLOPENING) {
                _statusType = STATUS_TYPE_CLOSE_WAIT_BOED_SOING;
            }

            if (status3 == TRADE_STATUS_BUYCLOSING) {
                _statusType = STATUS_TYPE_CLOSE_WAIT_BOED__BCING;
            }

            if (status3 == TRADE_STATUS_NOTHING) {
                _statusType = STATUS_TYPE_CLOSE_WAIT_BOED__NO;
            }

            break;

        case TRADE_STATUS_BUYOPENING:

            _statusType = STATUS_TYPE_CLOSE_BOING;

            if (status2 == TRADE_STATUS_SELLOPENED) {
                _statusType = STATUS_TYPE_CLOSE_WAIT_BOING_SOED;
            }

            if (status2 == TRADE_STATUS_SELLOPENING) {
                _statusType = STATUS_TYPE_CLOSE_BOING_SOING;
            }
            if (status2 == TRADE_STATUS_NOTHING) {
                _statusType = STATUS_TYPE_CLOSE_BOING_NO;
            }

            if (status3 == TRADE_STATUS_BUYCLOSING) {
                _statusType = STATUS_TYPE_CLOSE_BOING__BCING;
            }
            if (status3 == TRADE_STATUS_NOTHING) {
                _statusType = STATUS_TYPE_CLOSE_BOING__NO;
            }
            break;

        case TRADE_STATUS_SELLOPENING:

            _statusType = STATUS_TYPE_CLOSE_SOING;

            if (status3 == TRADE_STATUS_SELLCLOSING) {
                _statusType = STATUS_TYPE_CLOSE_SOING__SCING;
            }

            if (status3 == TRADE_STATUS_NOTHING) {
                _statusType = STATUS_TYPE_CLOSE_BOING__NO;
            }
            break;

        case TRADE_STATUS_BUYCLOSING:
            _statusType = STATUS_TYPE_CLOSE_BCING;
            break;

        case TRADE_STATUS_SELLCLOSING:
            _statusType = STATUS_TYPE_CLOSE_SCING;
            break;

        default:
            break;
    }
    _onKLineCloseDo(tick);
}

void TradeLogic::onKLineCloseByMe(KLineBlock block, TickData tick)
{
    _tick(tick);
    _kIndex = block.getIndex();
    if (!_isTradingTime(tick)) return;

    _isLock = true;
    int status1 = _getStatus(1);
    int status2 = _getStatus(2);
    int status3 = _getStatus(3);

    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "TradeLogicSrv[onKLineCloseByMe]";
    info << "|status1|" << status1;
    info << "|status2|" << status2;
    info << "|status3|" << status3;
    info << endl;
    info.close();

    switch (status1) {

        case TRADE_STATUS_NOTHING:
            _statusType = STATUS_TYPE_MYCLOSE_NO;
            if (status2 == TRADE_STATUS_SELLOPENED) {
                _setStatus(1, TRADE_STATUS_SELLOPENED);
                _statusType = STATUS_TYPE_MYCLOSE_NO_SOED;
            }
            break;

        case TRADE_STATUS_BUYOPENING:
            if (status2 == TRADE_STATUS_SELLOPENED) {
                _statusType = STATUS_TYPE_MYCLOSE_BOING_SOED;
            }
            break;

        case TRADE_STATUS_SELLOPENED:
            _statusType = STATUS_TYPE_MYCLOSE_SOED;
            if (status3 == TRADE_STATUS_NOTHING) {
                _statusType = STATUS_TYPE_MYCLOSE_SOED__NO;
            }
            if (status3 == TRADE_STATUS_SELLCLOSING) {
                _statusType = STATUS_TYPE_MYCLOSE_SOED__SCING;
            }
            break;

        case TRADE_STATUS_BUYOPENED:
            _statusType = STATUS_TYPE_MYCLOSE_BOED;

            if (status3 == TRADE_STATUS_NOTHING) {
                _statusType = STATUS_TYPE_MYCLOSE_BOED__NO;
            }
            if (status3 == TRADE_STATUS_BUYCLOSING) {
                _statusType = STATUS_TYPE_MYCLOSE_BOED__BCING;
            }
            if (status2 == TRADE_STATUS_SELLOPENING) {
                _statusType = STATUS_TYPE_MYCLOSE_BOED_SOING;
            }
            if (status2 == TRADE_STATUS_NOTHING) {
                _statusType = STATUS_TYPE_MYCLOSE_BOED_NO;
            }
            break;

        default:
            break;
    }
    _onKLineCloseDo(tick);

}

void TradeLogic::onTick(TickData tick)
{
    string flgStr = _store->get("NEED_TICK_" + _instrumnetID);
    int flg = Lib::stoi(flgStr);
    if (flg == 1)
        _onKLineCloseDo(tick);
}

void TradeLogic::_setTickSwitch(bool needTick)
{
    string flg = needTick ? "1" : "0";
    _store->set("NEED_TICK_" + _instrumnetID, flg);
}

void TradeLogic::_onKLineCloseDo(TickData tick)
{
    switch (_statusType) {
        case STATUS_TYPE_CLOSE_NO:
        case STATUS_TYPE_CLOSE_BOED:
        case STATUS_TYPE_CLOSE_SOED:
            _realAction(tick);
            break;
        case STATUS_TYPE_MYCLOSE_NO_SOED:
        case STATUS_TYPE_MYCLOSE_BOED_NO:
        case STATUS_TYPE_MYCLOSE_NO:
        case STATUS_TYPE_MYCLOSE_SOED__NO:
        case STATUS_TYPE_MYCLOSE_BOED__NO:
        case STATUS_TYPE_MYCLOSE_BOED:
        case STATUS_TYPE_MYCLOSE_SOED:
        case STATUS_TYPE_CLOSE_WAIT_BOED:
        case STATUS_TYPE_CLOSE_WAIT_SOED:
        case STATUS_TYPE_CLOSE_WAIT_NO:
        case STATUS_TYPE_CLOSE_WAIT_SOED__NO:
        case STATUS_TYPE_CLOSE_WAIT_BOED__NO:
            _forecast(tick);
            break;

        case STATUS_TYPE_CLOSE_BOING:
        case STATUS_TYPE_CLOSE_SOING:
        case STATUS_TYPE_CLOSE_BOING_SOING:
        case STATUS_TYPE_CLOSE_NO_SOING:
        case STATUS_TYPE_CLOSE_BOING_NO:
        case STATUS_TYPE_CLOSE_SOING__NO:
        case STATUS_TYPE_CLOSE_BOING__NO:
            if (_canRollbackOpen(tick)) {
                _setTickSwitch(false);
                _rollback();
            } else {
                _setTickSwitch(true);
            }
            break;

        case STATUS_TYPE_CLOSE_BOING__BCING:
            if (_canRollbackClose(tick, true) && _canRollbackOpen(tick)) {
                _setTickSwitch(false);
                _rollback();
            } else {
                _setTickSwitch(true);
            }

            break;

        case STATUS_TYPE_CLOSE_SOING__SCING:
            if (_canRollbackClose(tick, false) && _canRollbackOpen(tick)) {
                _setTickSwitch(false);
                _rollback();
            } else {
                _setTickSwitch(true);
            }
            break;

        case STATUS_TYPE_CLOSE_BCING:
            if (_canRollbackClose(tick, true)) {
                _setTickSwitch(false);
                _rollback();
            } else {
                _setTickSwitch(true);
            }
            break;

        case STATUS_TYPE_CLOSE_SCING:
            if (_canRollbackClose(tick, false)) {
                _setTickSwitch(false);
                _rollback();
            } else {
                _setTickSwitch(true);
            }
            break;

        case STATUS_TYPE_MYCLOSE_BOED_SOING:
        case STATUS_TYPE_MYCLOSE_BOING_SOED:
        case STATUS_TYPE_MYCLOSE_SOED__SCING:
        case STATUS_TYPE_MYCLOSE_BOED__BCING:
        case STATUS_TYPE_CLOSE_WAIT_BOED_SOING:
        case STATUS_TYPE_CLOSE_WAIT_BOING_SOED:
        case STATUS_TYPE_CLOSE_WAIT_SOED__SCING:
        case STATUS_TYPE_CLOSE_WAIT_BOED__BCING:
            _rollback();
            break;
        default:
            break;
    }
}


void TradeLogic::onRollback()
{
    int status1 = _getStatus(1);
    int status2 = _getStatus(2);
    int status3 = _getStatus(3);
    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "TradeLogicSrv[onRollback]";
    info << "|statusType|" << _statusType;
    info << "|status1|" << status1;
    info << "|status2|" << status2;
    info << "|status3|" << status3;
    info << endl;
    info.close();

    TickData tick = _getTick();

    if (_isTradeEnd) {
        _endClose();
        return;
    }

    switch (_statusType) {
        case STATUS_TYPE_CLOSE_SOING__NO:
        case STATUS_TYPE_CLOSE_BOING__NO:
            if (status1 == TRADE_STATUS_NOTHING) {
                _realAction(_closeTick);
            } else {
                _forecast(_closeTick);
            }
            break;
        case STATUS_TYPE_CLOSE_NO_SOING:
            if (status2 == TRADE_STATUS_NOTHING) {
                _realAction(_closeTick);
            } else {
                _setStatus(1, TRADE_STATUS_SELLOPENED);
                _forecast(_closeTick);
            }
            break;
        case STATUS_TYPE_CLOSE_BOING_NO:
            if (status1 == TRADE_STATUS_NOTHING) {
                _realAction(_closeTick);
            } else {
                _forecast(_closeTick);
            }

        case STATUS_TYPE_MYCLOSE_BOING_SOED:
        case STATUS_TYPE_CLOSE_WAIT_BOING_SOED:
            if (status1 == TRADE_STATUS_NOTHING) {
                _setStatus(1, TRADE_STATUS_SELLOPENED);
                _forecast(_closeTick);
            } else {
                // 不知道怎么办。。。
            }
            break;

        case STATUS_TYPE_MYCLOSE_BOED_SOING:
        case STATUS_TYPE_CLOSE_WAIT_BOED_SOING:
            if (status2 == TRADE_STATUS_NOTHING) {
                _forecast(_closeTick);
            } else {
                // 不知道怎么办。。。
            }
            break;

        case STATUS_TYPE_CLOSE_BOING_SOING:
            if (status1 == TRADE_STATUS_NOTHING && status2 == TRADE_STATUS_NOTHING) {
                _realAction(_closeTick);
            }
            if (status1 == TRADE_STATUS_NOTHING && status2 == TRADE_STATUS_SELLOPENED) {
                _setStatus(1, TRADE_STATUS_SELLOPENED);
                _forecast(_closeTick);
            }
            if (status2 == TRADE_STATUS_NOTHING && status1 == TRADE_STATUS_BUYOPENED) {
                _forecast(_closeTick);
            }
            if (status1 == TRADE_STATUS_BUYOPENED && status2 == TRADE_STATUS_SELLOPENED) {
                // 不知道怎么处理 TODO
            }
            break;

        case STATUS_TYPE_CLOSE_SCING:
        case STATUS_TYPE_CLOSE_BCING:
            if (status1 == TRADE_STATUS_NOTHING) {
                _forecast(_closeTick);
            } else {
                _realAction(_closeTick);
            }
            break;

        case STATUS_TYPE_MYCLOSE_SOED__SCING:
        case STATUS_TYPE_CLOSE_WAIT_SOED__SCING:
            if (status3 == TRADE_STATUS_BUYOPENED) {
                _sendMsg(MSG_TRADE_SELLCLOSE, tick.askPrice1, false, 0, 3);
            } else {
                _forecast(_closeTick);
            }
            break;

        case STATUS_TYPE_MYCLOSE_BOED__BCING:
        case STATUS_TYPE_CLOSE_WAIT_BOED__BCING:
            if (status3 == TRADE_STATUS_SELLOPENED) {
                _sendMsg(MSG_TRADE_BUYCLOSE, tick.bidPrice1, false, 0, 3);
            } else {
                _forecast(_closeTick);
            }
            break;

        case STATUS_TYPE_CLOSE_SOING__SCING:
            if (status1 == TRADE_STATUS_NOTHING && status3 == TRADE_STATUS_BUYOPENED) {
                _setStatus(1, TRADE_STATUS_BUYOPENED);
                _realAction(_closeTick);
            }
            if (status1 == TRADE_STATUS_NOTHING && status3 == TRADE_STATUS_NOTHING) {
                _realAction(_closeTick);
            }
            if (status1 == TRADE_STATUS_SELLOPENED && status3 == TRADE_STATUS_BUYOPENED) {
                _sendMsg(MSG_TRADE_SELLCLOSE, tick.askPrice1, false, 0, 3);
            }
            if (status1 == TRADE_STATUS_SELLOPENED && status3 == TRADE_STATUS_NOTHING) {
                _forecast(_closeTick);
            }
            break;

        case STATUS_TYPE_CLOSE_BOING__BCING:
            if (status1 == TRADE_STATUS_NOTHING && status3 == TRADE_STATUS_SELLOPENED) {
                _setStatus(1, TRADE_STATUS_SELLOPENED);
                _realAction(_closeTick);
            }
            if (status1 == TRADE_STATUS_NOTHING && status3 == TRADE_STATUS_NOTHING) {
                _realAction(_closeTick);
            }
            if (status1 == TRADE_STATUS_BUYOPENED && status3 == TRADE_STATUS_SELLOPENED) {
                _sendMsg(MSG_TRADE_BUYCLOSE, tick.bidPrice1, false, 0, 3);
            }
            if (status1 == TRADE_STATUS_BUYOPENED && status3 == TRADE_STATUS_NOTHING) {
                _forecast(_closeTick);
            }
            break;

        case STATUS_TYPE_CLOSE_BOING:
        case STATUS_TYPE_CLOSE_SOING:
            _realAction(_closeTick);
            break;

        default:
            break;
    }

}

void TradeLogic::onRealActionBack()
{
    int status1 = _getStatus(1);
    int status2 = _getStatus(2);
    int status3 = _getStatus(3);
    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "TradeLogicSrv[onRealActionBack]";
    info << "|status1|" << status1;
    info << "|status2|" << status2;
    info << "|status3|" << status3;
    info << endl;
    info.close();

    if (_isTradeEnd) {
        return;
    }

    _forecast(_closeTick);
}

TickData TradeLogic::_getTick()
{
    string tickStr = _store->get("CURRENT_TICK_" + _instrumnetID);
    return Lib::string2TickData(tickStr);
}

int TradeLogic::_getStatus(int way)
{
    string status = _store->get("TRADE_STATUS_" + Lib::itos(way) + "_" + _instrumnetID);
    return Lib::stoi(status);
}

void TradeLogic::_setStatus(int way, int status)
{
    _store->set("TRADE_STATUS_" + Lib::itos(way) + "_" + _instrumnetID, Lib::itos(status));
}


void TradeLogic::_sendMsg(int msgType, double price, bool isForecast, int forecastID, int statusWay, bool isFok)
{

    int kIndex = _kIndex;
    if (isForecast) kIndex++;
    _actionKIndex = kIndex;
    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "TradeLogicSrv[sendMsg]";
    info << "|forecastID|" << forecastID;
    info << "|isForecast|" << isForecast;
    info << "|statusWay|" << statusWay;
    info << "|isFok|" << isFok;
    info << "|action|" << msgType;
    info << "|price|" << price;
    info << "|kIndex|" << kIndex << endl;
    info.close();

    MSG_TO_TRADE_STRATEGY msg = {0};
    msg.msgType = msgType;
    msg.price = price;
    msg.kIndex = kIndex;
    msg.total = 1;
    msg.isForecast = isForecast;
    if (isForecast) msg.forecastID = forecastID;
    msg.statusWay = statusWay;
    msg.isFok = isFok;
    strcpy(msg.instrumnetID, Lib::stoc(_instrumnetID));
    _tradeStrategySrvClient->send((void *)&msg);

}

void TradeLogic::_sendRollBack(int forecastID)
{
    //log
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "TradeLogicSrv[sendRollBack]";
    info << "|forecastID|" << forecastID;
    info << endl;
    info.close();

    MSG_TO_TRADE_STRATEGY msg = {0};
    msg.msgType = MSG_TRADE_ROLLBACK;
    msg.forecastID = forecastID;
    strcpy(msg.instrumnetID, Lib::stoc(_instrumnetID));
    _tradeStrategySrvClient->send((void *)&msg);

}
