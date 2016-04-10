#include "KLineBlock.h"

KLineBlock::KLineBlock(int index, string date, string time, 
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

KLineBlock::~KLineBlock()
{

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

void KLineBlock::show()
{
    cout << "[" << _index << "]";
    cout << "[" << _type << "]";
    cout << "[" << _openDate << "]";
    cout << "[" << _openTime << "]";
    cout << "[" << _openPrice << "]";
    cout << "[" << _maxPrice << "]";
    cout << "[" << _minPrice << "]";
    cout << "[" << _closePrice << "]";
    cout << "[" << _volume << "]";
    cout << "[" << _closeDate << "]";
    cout << "[" << _closeTime << "]";
    cout << endl;
}