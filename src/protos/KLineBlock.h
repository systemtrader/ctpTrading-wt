#ifndef K_LINE_BLOCK_H
#define K_LINE_BLOCK_H

#include "../libs/Lib.h"
#include "Data.h"
#include <string>
#include <iostream>
#include <vector>

using namespace std;

#define KLINE_TYPE_UNKOWN 0
#define KLINE_TYPE_UP 1
#define KLINE_TYPE_DOWN 2

class KLineBlock
{
private:

    int _index;
    int _type;

    double _maxPrice;
    double _minPrice;
    double _openPrice;
    double _closePrice;
    int _openMsec;
    int _closeMsec;

    string _openDate;
    string _openTime;
    string _closeDate;
    string _closeTime;
    int _volume;


public:
    KLineBlock();
    ~KLineBlock();

    void init(int, TickData);
    void update(TickData);
    void close();
    void show();
    KLineBlockData exportData();
    string exportString();

    double getMaxPrice();
    double getMinPrice();
    double getOpenPrice();
    double getClosePrice();
    string getOpenDate();
    string getOpenTime();
    string getCloseDate();
    string getCloseTime();
    int getOpenMsec();
    int getType();
    int getIndex();
    int getVolume();
    void setVal(string);
    string getVal();

    static KLineBlock makeViaData(KLineBlockData);
};
#endif
