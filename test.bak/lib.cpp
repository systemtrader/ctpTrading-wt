#include "config.h"
#include <time.h>
#include <cstring>
#include <fstream>
#include "lib.h"

/**
 * 获取当前时间
 * @param  format [description]
 * @return        [description]
 */
string getDate(string format)
{
    time_t t = time(0);
    char tmp[20];
    strftime(tmp, sizeof(tmp), format.c_str(), localtime(&t));
    string s(tmp);
    return s;
}

string getPath(const string pathName, int type = PATH_LOG)
{
    string date, root;
    switch (type) {
        case PATH_LOG:
            date = getDate("%Y%m%d");
            return LOG_PATH + pathName + "_" + date + ".log";
        case PATH_PID:
            return APP_ROOT + "/pid";
        case PATH_DATA:
            date = getDate("%Y%m%d");
            return DATA_PATH + pathName + "_" + date + ".log";
        default:
            break;
    }
    return "";
}

char * stoc(string str)
{
    const char * s = str.c_str();
    char * ch = new char[strlen(s) + 1];
    strcpy(ch, s);
    return ch;
}

void sysErrLog(string logName, CThostFtdcRspInfoField *info, int id, int isLast)
{
    string sysPath = getPath("sys");
    ofstream sysLogger;
    sysLogger.open(sysPath.c_str(), ios::app);

    sysLogger << getDate(LOG_TIMESTAMP_FORMAT) << LOG_SPLIT;
    sysLogger << "[ERROR]" << LOG_SPLIT;
    sysLogger << logName << LOG_SPLIT;
    sysLogger << "ErrCode" << LOG_SPLIT << info->ErrorID << LOG_SPLIT;
    sysLogger << "ErrMsg" << LOG_SPLIT << info->ErrorMsg << LOG_SPLIT;
    sysLogger << "RequestID" << LOG_SPLIT << id << LOG_SPLIT;
    sysLogger << "IsLast" << LOG_SPLIT << isLast << endl;

    sysLogger.close();
}

void sysReqLog(string logName, int code)
{
    string sysPath = getPath("sys");
    ofstream sysLogger;
    sysLogger.open(sysPath.c_str(), ios::app);

    sysLogger << getDate(LOG_TIMESTAMP_FORMAT) << LOG_SPLIT;
    sysLogger << "[REQUEST]" << LOG_SPLIT;
    sysLogger << logName << LOG_SPLIT;
    sysLogger << "Code" << LOG_SPLIT << code << endl;

    sysLogger.close();
}
