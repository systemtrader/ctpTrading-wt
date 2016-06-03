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
    }
}


KLineSrv::~KLineSrv()
{
    cout << "~KLineSrv" << endl;
}

void KLineSrv::onTickCome(TickData tick)
{
    if (_isBlockExist()) {
        _updateBlock(tick);
        if (_checkBlockClose(tick)) {
            _closeBlock(tick);
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
    Lib::initInfoLogHandle(_logPath, info);
    info << "KLineSrv[open]";
    info << "|iID|" << _instrumnetID;
    info << "|index|" << _index << endl;
    info.close();

    // 发送消息
    MSG_TO_TRADE_LOGIC msg = {0};
    msg.msgType = MSG_KLINE_OPEN;
    msg.block = _currentBlock->exportData();
    msg.tick = tick;
    _tradeLogicSrvClient->send((void *)&msg);

    _index++;
}

bool KLineSrv::_checkBlockClose(TickData tick)
{
    if (abs(_currentBlock->getOpenPrice() - tick.price) >= _kRange) {
        return true;
    }
    return false;
}

void KLineSrv::_updateBlock(TickData tick)
{
    _currentBlock->update(tick);
}

void KLineSrv::_closeBlock(TickData tick)
{
    
    _currentBlock->close();
    string keyQ = "K_LINE_Q";
    string strData = _currentBlock->exportString();
    KLineBlockData blockData = _currentBlock->exportData();
    delete _currentBlock;
    _currentBlock = NULL;

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "KLineSrv[close]";
    info << "|iID|" << _instrumnetID;
    info << "|index|" << blockData.index;
    info << "|open|" << blockData.open;
    info << "|close|" << blockData.close;
    info << endl;
    info.close();
    
    // 发送消息
    MSG_TO_TRADE_LOGIC msg = {0};
    msg.msgType = MSG_KLINE_CLOSE;
    msg.block = blockData;
    msg.tick = tick;
    _tradeLogicSrvClient->send((void *)&msg);

    // 储存消息
    _store->push(keyQ, strData);

}
