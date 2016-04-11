#include "KLineSrv.h"
#include "../../libs/Lib.h"
#include <cmath>

KLineSrv::KLineSrv(int kRange)
{
    _index = 0;
    _kRange = kRange;
    _currentBlock = NULL;
    _store = new Redis("127.0.0.1", 6379, 1);
}

KLineSrv::~KLineSrv()
{
    delete _store;
    cout << "~KLineSrv" << endl;
}

void KLineSrv::onTickCome(Tick tick)
{
    if (_isBlockExist()) {
        _updateBlock(tick);
        if (_checkBlockClose(tick)) {
            _closeBlock(tick);
        }
    } else {
        _initBlock(tick);
    }
    _currentBlock->show();
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
    _index++;
    // 发送消息TODO
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
    _store->push(keyQ, storeData);
}
