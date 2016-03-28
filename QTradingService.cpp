#include "QTradingService.h"
#include <iostream>

using namespace std;

QTradingService::QTradingService(int env)
{
    _env = env;
    cout << "constructor" << endl;
}

QTradingService::~QTradingService()
{
    cout << "~" << endl;
}

void QTradingService::init()
{
    cout << "init" << endl;
}

void QTradingService::start()
{
    cout << "start..." << endl;
    while (true) {
        cout << "wait..." << endl;
        sleep(1);
    }
}

void QTradingService::stop()
{
    cout << "stop" << endl;
}
