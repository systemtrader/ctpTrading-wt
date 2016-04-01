#ifndef LIB_H
#define LIB_H

#include "ThostFtdcUserApiStruct.h"
#include <string>
using namespace std;

string getDate(string format);
string getPath(const string, int);
void sysErrLog(string logName, CThostFtdcRspInfoField *info, int id, int isLast);
void sysReqLog(string logName, int res);
char * stoc(string);
#endif
