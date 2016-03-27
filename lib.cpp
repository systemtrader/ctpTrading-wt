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

string getLogPath(const string & pathName)
{
    string date = getDate("%Y%m%d");
    return "./log/" + pathName + "_" + date + ".log";
}

