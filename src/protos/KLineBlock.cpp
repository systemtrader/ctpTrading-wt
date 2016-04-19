#include "KLineBlock.h"

KLineBlock::KLineBlock()
{

}

KLineBlock::~KLineBlock()
{

}

void KLineBlock::init(int index, string date, string time,
    double tick, int volume)
{
    _index = index;
    _openDate = date;
    _openTime = time;

    _maxPrice = tick;
    _minPrice = tick;
    _openPrice = tick;
    _closePrice = tick;

    _volume = volume;

    _type = KLINE_TYPE_UNKOWN;
}

void KLineBlock::update(double tick, int volume)
{
    if (tick > _maxPrice) {
        _maxPrice = tick;
    }
    if (tick < _minPrice) {
        _minPrice = tick;
    }
    _closePrice = tick;
}

void KLineBlock::close(string date, string time)
{
    _closeDate = date;
    _closeTime = time;
    _type = _openPrice > _closePrice ? KLINE_TYPE_UP : KLINE_TYPE_DOWN;
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

KLineBlock KLineBlock::makeSimple(string index, string type, string openPrice,
    string maxPrice, string minPrice, string closePrice, string volume)
{
    KLineBlock block;
    block._index      = Lib::stoi(index);
    block._type       = Lib::stoi(type);
    block._openPrice  = Lib::stod(openPrice);
    block._maxPrice   = Lib::stod(maxPrice);
    block._minPrice   = Lib::stod(minPrice);
    block._closePrice = Lib::stod(closePrice);
    block._volume     = Lib::stoi(volume);
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
