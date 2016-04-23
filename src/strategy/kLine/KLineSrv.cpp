#include "KLineSrv.h"

KLineSrv::KLineSrv(int kRange, int serviceID, string logPath)
{
    _index = 0;
    _kRange = kRange;
    _logPath = logPath;
    _currentBlock = NULL;

    _store = new Redis("127.0.0.1", 6379, 1);
    _tradeLogicSrvClient = new QClient(serviceID, sizeof(MSG_TO_TRADE_LOGIC));
}

KLineSrv::~KLineSrv()
{
    // delete _store;
    // delete _tradeLogicSrvClient;
    cout << "~KLineSrv" << endl;
}

void KLineSrv::onTickCome(TickData tick)
{
    if (_isBlockExist()) {
        _updateBlock(tick);
        _currentBlock->show();
        if (_checkBlockClose(tick)) {
            _closeBlock(tick);
        }
    } else {
        _initBlock(tick);
        _currentBlock->show();
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
