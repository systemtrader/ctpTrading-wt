#include "KLineBlock.h"

KLineBlock::KLineBlock()
{

}

KLineBlock::~KLineBlock()
{

}

void KLineBlock::setVal(string str)
{
    std::vector<string> params = Lib::split(str, "_");
    _index = Lib::stoi(params[0]);
    _openPrice = Lib::stod(params[1]);
    _openDate = params[2];
    _openTime = params[3];
    _openMsec = Lib::stoi(params[4]);
    _maxPrice = Lib::stod(params[5]);
    _minPrice = Lib::stod(params[6]);
    _type = KLINE_TYPE_UNKOWN;
}

string KLineBlock::getVal()
{
    string currentStr = Lib::itos(_index) + "_" +
                        Lib::dtos(_openPrice) + "_" +
                        _openDate + "_" +
                        _openTime + "_" +
                        Lib::itos(_openMsec) + "_" +
                        Lib::dtos(_maxPrice) + "_" +
                        Lib::dtos(_minPrice);
    return currentStr;
}

void KLineBlock::init(int index, TickData tick)
{
    _index = index;
    _openDate = string(tick.date);
    _openTime = string(tick.time);
    _openMsec = tick.msec;

    _maxPrice = _minPrice = _openPrice = _closePrice = tick.price;
    _volume = tick.volume;

    _type = KLINE_TYPE_UNKOWN;
}

void KLineBlock::update(TickData tick)
{
    if (tick.price > _maxPrice) {
        _maxPrice = tick.price;
    }
    if (tick.price < _minPrice) {
        _minPrice = tick.price;
    }
    _closePrice = tick.price;
    _closeDate = string(tick.date);
    _closeTime = string(tick.time);
    _closeMsec = tick.msec;
}

void KLineBlock::close()
{
    _type = _openPrice > _closePrice ? KLINE_TYPE_UP : KLINE_TYPE_DOWN;
}

KLineBlockData KLineBlock::exportData()
{
    KLineBlockData block = {0};
    block.index = _index;
    block.open = _openPrice;
    block.max = _maxPrice;
    block.min = _minPrice;
    block.close = _closePrice;
    return block;
}

string KLineBlock::exportString()
{
    string str = Lib::itos(_index) + "_" +
                 Lib::itos(_type) + "_" +
                 _openDate + "_" +
                 _openTime + "_" +
                 Lib::itos(_openMsec) + "_" +
                 Lib::dtos(_openPrice) + "_" +
                 Lib::dtos(_maxPrice) + "_" +
                 Lib::dtos(_minPrice) + "_" +
                 Lib::dtos(_closePrice) + "_" +
                 Lib::itos(_volume) + "_" +
                 _closeDate + "_" +
                 _closeTime + "_" +
                 Lib::itos(_closeMsec);
    return str;
}

double KLineBlock::getMaxPrice()
{
    return _maxPrice;
}

double KLineBlock::getMinPrice()
{
    return _minPrice;
}

double KLineBlock::getOpenPrice()
{
    return _openPrice;
}

double KLineBlock::getClosePrice()
{
    return _closePrice;
}

string KLineBlock::getOpenDate()
{
    return _openDate;
}

string KLineBlock::getOpenTime()
{
    return _openTime;
}

string KLineBlock::getCloseDate()
{
    return _closeDate;
}

string KLineBlock::getCloseTime()
{
    return _closeTime;
}

int KLineBlock::getIndex()
{
    return _index;
}

int KLineBlock::getType()
{
    return _type;
}

int KLineBlock::getVolume()
{
    return _volume;
}

int KLineBlock::getOpenMsec()
{
    return _openMsec;
}

KLineBlock KLineBlock::makeViaData(KLineBlockData blockData)
{
    KLineBlock block;
    block._index      = blockData.index;
    block._openPrice  = blockData.open;
    block._maxPrice   = blockData.max;
    block._minPrice   = blockData.min;
    block._closePrice = blockData.close;
    return block;
}

void KLineBlock::show()
{
    cout << " index:[" << _index << "]";
    cout << " type:[" << _type << "]";
    cout << " openDate[" << _openDate << "]";
    cout << " openTime[" << _openTime << "]";
    cout << " open[" << _openPrice << "]";
    cout << " max[" << _maxPrice << "]";
    cout << " min[" << _minPrice << "]";
    cout << " close[" << _closePrice << "]";
    cout << " volume[" << _volume << "]";
    cout << " closeDate[" << _closeDate << "]";
    cout << " closeTime[" << _closeTime << "]";
    cout << endl;
}
