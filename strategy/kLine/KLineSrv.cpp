#include "KLineSrv.h"
#include "../../libs/Lib.h"
#include <cmath>

KLineSrv::KLineSrv(int KRange)
{
    _index = 0;
    _kRange = KRange;
    _currentBlock = NULL;
    _store = new Redis("127.0.0.1", 6379, 1);
}

KLineSrv::~KLineSrv()
{
    delete _store;
    if (_currentBlock)
        delete _currentBlock;
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
    if (_currentBlock == NULL) return 0;
    return 1;
}

void KLineSrv::_initBlock(Tick tick)
{
    _currentBlock = new KLineBlock(_index, tick.date, tick.time,
        tick.price, tick.volume);
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
    string localTime = Lib::getDate("%Y-%m-%d %H:%M:%S");
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
    _store->push(keyQ, storeData);
}
