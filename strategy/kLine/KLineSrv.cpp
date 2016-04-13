#include "KLineSrv.h"
#include "../../libs/Lib.h"
#include "../../libs/Socket.h"
#include "../../iniReader/iniReader.h"
#include <cmath>

KLineSrv::KLineSrv(int kRange)
{
    _index = 0;
    _kRange = kRange;
    _currentBlock = NULL;
    _store = new Redis("127.0.0.1", 6379, 1);
    int          port = getOptionToInt("logic_front_srv_port");
    const char * ip   = getOptionToChar("logic_front_srv_ip");

    _msgFD = getCSocket(ip, port);
}

KLineSrv::~KLineSrv()
{
    close(_msgFD);
    delete _store;
    cout << "~KLineSrv" << endl;
}

void KLineSrv::onTickCome(Tick tick)
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

int KLineSrv::_isBlockExist()
{
    if (NULL == _currentBlock) return 0;
    return 1;
}

void KLineSrv::_initBlock(Tick tick)
{
    _currentBlock = new KLineBlock();
    _currentBlock->init(_index, tick.date, tick.time,
        tick.price, tick.volume);
    // 发送消息TODO
    string msgData = Lib::itos(_index) + "_" +
                    Lib::dtos(tick.price) + "_" +
                    Lib::itos(tick.volume);
    string msg = CMD_MSG_KLINE_OPEN + "_" + msgData;
    sendMsg(_msgFD, msg);
    _index++;
}

int KLineSrv::_checkBlockClose(Tick tick)
{
    if (abs(_currentBlock->getOpenPrice() - tick.price) > _kRange) {
        return 1;
    }
    return 0;
}

void KLineSrv::_updateBlock(Tick tick)
{
    _currentBlock->update(tick.price, tick.volume);
}

void KLineSrv::_closeBlock(Tick tick)
{
    _currentBlock->close(tick.date, tick.time);
    string localTime = Lib::getDate("%Y%m%d-%H:%M:%S");
    string keyQ = "K_LINE_Q";
    string storeData = localTime + "_" +
                    Lib::itos(_currentBlock->getIndex()) + "_" +
                    Lib::itos(_currentBlock->getType()) + "_" +
                    _currentBlock->getOpenDate() + "_" +
                    _currentBlock->getOpenTime() + "_" +
                    Lib::dtos(_currentBlock->getOpenPrice()) + "_" +
                    Lib::dtos(_currentBlock->getMaxPrice()) + "_" +
                    Lib::dtos(_currentBlock->getMinPrice()) + "_" +
                    Lib::dtos(_currentBlock->getClosePrice()) + "_" +
                    Lib::itos(_currentBlock->getVolume()) + "_" +
                    _currentBlock->getCloseDate() + "_" +
                    _currentBlock->getCloseTime();
    delete _currentBlock;
    _currentBlock = NULL;
    // 发送消息
    string msg = CMD_MSG_KLINE_CLOSE + "_" + storeData;
    sendMsg(_msgFD, msg);
    // 储存消息
    _store->push(keyQ, storeData);
}
