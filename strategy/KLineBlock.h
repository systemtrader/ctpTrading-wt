#ifndef K_LINE_BLOCK_H
#define K_LINE_BLOCK_H
#include <string>
#include <iostream>

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

    string _openDate;
    string _openTime;
    string _closeDate;
    string _closeTime;
    int _volume;


public:
    KLineBlock();
    ~KLineBlock();

    void init(int index, string date, string time,
        double tick, int volume);
    void update(double tick, int volume);
    void close(string date, string time);

    double getMaxPrice();
    double getMinPrice();
    double getOpenPrice();
    double getClosePrice();
    string getOpenDate();
    string getOpenTime();
    string getCloseDate();
    string getCloseTime();
    int getType();
    int getIndex();
    int getVolume();

    void show();
};
#endif
