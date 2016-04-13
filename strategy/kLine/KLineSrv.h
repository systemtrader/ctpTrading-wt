#ifndef K_LINE_SRV_H
#define K_LINE_SRV_H
#include "../KLineBlock.h"
#include "../Tick.h"
#include "../../libs/Redis.h"

class KLineSrv
{
private:

    KLineBlock * _currentBlock;
    Redis * _store;
    int _msgFD;

    int _index;
    int _kRange;

    int _isBlockExist();
    int _checkBlockClose(Tick tick);
    void _initBlock(Tick tick);
    void _updateBlock(Tick tick);
    void _closeBlock(Tick tick);

public:
    KLineSrv(int kRange);
    ~KLineSrv();

    void onTickCome(Tick tick);
};
#endif
