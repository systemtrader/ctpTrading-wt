#ifndef LIB_H
#define LIB_H

#include "../ThostFtdcUserApiStruct.h"
#include "../iniReader/iniReader.h"
#include <string>
#include <time.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

const int PATH_LOG  = 1;
const int PATH_PID  = 2;
const int PATH_DATA = 3;

class Lib
{
public:

    static string getDate(string format);

    static string getPath(const string pathName, int type);

    static char * stoc(string str);

    static string dtos(double dbl);

    static int stoi(string s);

    static string itos(int num);

    static void sysErrLog(string logName, CThostFtdcRspInfoField *info, int id, int isLast);

    static void sysReqLog(string logName, int code);

    static void initInfoLogHandle(ofstream & sysLogger);

    static vector<string> split(const string& s, const string& delim);

    static double max(double arr[], int cnt);
    static double min(double arr[], int cnt);
    static double mean(double arr[], int cnt);

    Lib();
    ~Lib();

};

#endif
