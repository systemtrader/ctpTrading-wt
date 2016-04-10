#include "KLineSrv.h"
#include <cmath>

KLineSrv::KLineSrv(int KRange)
{
    _index = 0;
    _kRange = KRange;
    _currentBlock = NULL;
}

KLineSrv::~KLineSrv()
{
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
}