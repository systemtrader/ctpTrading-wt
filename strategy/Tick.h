#ifndef TICK_H
#define TICK_H

#include <string>

using namespace std;

struct Tick
{
    double price;
    int volume;
    double bidPrice1;
    double askPrice1;
    string date;
    string time;
};


#endif
