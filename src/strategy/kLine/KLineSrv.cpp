#include "KLineSrv.h"

KLineSrv::KLineSrv(int kRange, int serviceID, string logPath, int db)
{
    _index = 0;
    _kRange = kRange;
    _logPath = logPath;
    _currentBlock = NULL;

    _store = new Redis("127.0.0.1", 6379, db);
    _tradeLogicSrvClient = new QClient(serviceID, sizeof(MSG_TO_TRADE_LOGIC));

    // 初始化数据
    string currentStr = _store->get("CURRENT_BLOCK_STORE");
    if (currentStr.length() > 0) { // 上次K线未关闭，初始化数据
        _currentBlock = new KLineBlock();
        _currentBlock->setVal(currentStr);
        _store->set("CURRENT_BLOCK_STORE", ""); // 清空记录
        _index = _currentBlock->getIndex() + 1;
    }
}



KLineSrv::~KLineSrv()
{
    // delete _store;
    // delete _tradeLogicSrvClient;
    if (_currentBlock->getType() == KLINE_TYPE_UNKOWN) {// k线未封闭则保存K线状态
        string currentStr = _currentBlock->getVal();
        _store->set("CURRENT_BLOCK_STORE", currentStr);
    }
    cout << "~KLineSrv" << endl;
}

void KLineSrv::onTickCome(TickData tick)
{
    if (_isBlockExist()) {
        _updateBlock(tick);
        if (_checkBlockClose(tick)) {
            _closeBlock(tick);
        } else {
            _transTick(tick);
        }
    } else {
        _initBlock(tick);
    }
}

void KLineSrv::_transTick(TickData tick)
{
    // 发送消息
    MSG_TO_TRADE_LOGIC msg = {0};
    msg.msgType = MSG_TICK;
    msg.tick = tick;
    _tradeLogicSrvClient->send((void *)&msg);
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

    // 发送消息
    MSG_TO_TRADE_LOGIC msg = {0};
    msg.msgType = MSG_KLINE_OPEN;
    msg.block = _currentBlock->exportData();
    _tradeLogicSrvClient->send((void *)&msg);

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "KLineSrv[open]";
    info << "|index|" << _index << endl;
    info.close();

    _index++;
}

bool KLineSrv::_checkBlockClose(TickData tick)
{
    if (abs(_currentBlock->getOpenPrice() - tick.price) > _kRange) {
        return true;
    }
    return false;
}

void KLineSrv::_updateBlock(TickData tick)
{
    _currentBlock->update(tick);
}

void KLineSrv::_closeBlock(Tick tick)
{
    _currentBlock->close();
    string keyQ = "K_LINE_Q";
    string strData = _currentBlock->exportString();
    KLineBlockData blockData = _currentBlock->exportData();
    delete _currentBlock;
    _currentBlock = NULL;

    // 发送消息
    MSG_TO_TRADE_LOGIC msg = {0};
    msg.msgType = MSG_KLINE_CLOSE;
    msg.block = blockData;
    _tradeLogicSrvClient->send((void *)&msg);

    // 储存消息
    _store->push(keyQ, strData);

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "KLineSrv[close]";
    info << "|index|" << blockData.index << endl;
    info.close();
}
