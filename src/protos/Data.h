#ifndef DATA_H
#define DATA_H

typedef struct Tick
{
    double price;
    int volume;
    double bidPrice1;
    double askPrice1;
    char date[9];
    char time[9];
    int msec;
} TickData;

typedef struct KLineBlockData
{
    int index;
    double open;
    double max;
    double min;
    double close;

} KLineBlockData;

#endif
