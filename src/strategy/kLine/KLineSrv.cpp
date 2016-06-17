#include "KLineSrv.h"

KLineSrv::KLineSrv(int kRange, int serviceID, string logPath, int db, string instrumnetID)
{
    _index = 0;
    _kRange = kRange;
    _logPath = logPath;
    _currentBlock = NULL;
    _instrumnetID = instrumnetID;

    _store = new Redis("127.0.0.1", 6379, db);
    _tradeLogicSrvClient = new QClient(serviceID, sizeof(MSG_TO_TRADE_LOGIC));

    // 初始化数据
    string currentStr = _store->get("CURRENT_BLOCK_STORE_" + instrumnetID);
    if (currentStr.length() > 0) { // 上次K线未关闭，初始化数据
        _currentBlock = new KLineBlock();
        _currentBlock->setVal(currentStr, instrumnetID);
        _index = _currentBlock->getIndex() + 1;
    } else {
        string iStr = _store->get("CURRENT_BLOCK_INDEX_" + instrumnetID);
        _index = Lib::stoi(iStr);
    }
}


KLineSrv::~KLineSrv()
{
    cout << "~KLineSrv" << endl;
}

void KLineSrv::onTickCome(TickData tick, bool isMy)
{
    if (isMy) {
        ofstream info;
        Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
        info << "KLineSrv[onMyTick]";
        info << "|open|" << _currentBlock->getOpenPrice();
        info << "|price|" << tick.price;
        info << "|range|" << _kRange;
        info << endl;
        info.close();
    }
    if (_isBlockExist()) {
        _updateBlock(tick);
        if (_checkBlockClose(tick, isMy)) {
            _closeBlock(tick, isMy);
            _initBlock(tick);// 两根K线的开闭共享一个tick
        }
    } else {
        _initBlock(tick);
    }
}

bool KLineSrv::_isBlockExist()
{
    if (NULL == _currentBlock) return false;
    return true;
}

void KLineSrv::_initBlock(TickData tick)
{
    _currentBlock = new KLineBlock();
    _currentBlock->init(_index, tick);


    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "KLineSrv[open]";
    info << "|index|" << _index;
    info << "|open|" << _currentBlock->getOpenPrice();
    info << endl;
    info.close();

    // // 发送消息
    // MSG_TO_TRADE_LOGIC msg = {0};
    // msg.msgType = MSG_KLINE_OPEN;
    // msg.block = _currentBlock->exportData();
    // msg.tick = tick;
    // _tradeLogicSrvClient->send((void *)&msg);

    _index++;
}

bool KLineSrv::_checkBlockClose(TickData tick, bool isMy)
{
    if (isMy) {
        ofstream info;
        Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
        info << "KLineSrv[checkBlockClose]";
        info << "|abc|" << abs(_currentBlock->getOpenPrice() - tick.price);
        info << "|open|" << _currentBlock->getOpenPrice();
        info << "|price|" << tick.price;
        info << "|range|" << _kRange;
        info << endl;
        info.close();
    }
    if (abs(_currentBlock->getOpenPrice() - tick.price) >= (double)_kRange - 0.1) {
        return true;
    }
    return false;
}

void KLineSrv::_updateBlock(TickData tick)
{
    _currentBlock->update(tick);
}

void KLineSrv::_closeBlock(TickData tick, bool isMy)
{

    _currentBlock->close();
    string keyQ = "K_LINE_Q";
    string strData = _currentBlock->exportString();
    KLineBlockData blockData = _currentBlock->exportData();
    delete _currentBlock;
    _currentBlock = NULL;

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info, _instrumnetID);
    info << "KLineSrv[close]";
    info << "|index|" << blockData.index;
    info << "|open|" << blockData.open;
    info << "|close|" << blockData.close;
    info << "|isMy|" << isMy;
    info << endl;
    info << endl;
    info << endl;
    info.close();

    // 发送消息
    MSG_TO_TRADE_LOGIC msg = {0};
    if (isMy) {
        msg.msgType = MSG_KLINE_CLOSE_BY_ME;
    } else {
        msg.msgType = MSG_KLINE_CLOSE;
    }
    msg.block = blockData;
    msg.tick = tick;
    _tradeLogicSrvClient->send((void *)&msg);

    // 储存消息
    _store->push(keyQ, strData);

}
