#include "Lib.h"


Lib::Lib(){};
Lib::~Lib(){};

string Lib::getDate(string format, bool needUsec)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    time_t t;
    t = tv.tv_sec;
    char tmp[20];
    strftime(tmp, sizeof(tmp), format.c_str(), localtime(&t));
    string s(tmp);
    if (needUsec) {
        s += "." + itos(tv.tv_usec);
    }
    return s;
};

char * Lib::stoc(string str)
{
    const char * s = str.c_str();
    char * ch = new char[strlen(s) + 1];
    strcpy(ch, s);
    return ch;
};

int Lib::stoi(string s)
{
    return atoi(s.c_str());
}

double Lib::stod(string s)
{
    return atof(s.c_str());
}

string Lib::dtos(double dbl)
{
    std::ostringstream strs;
    strs << dbl;
    std::string str = strs.str();
    return str;
};

string Lib::itos(int num)
{
    char s[10];
    sprintf(s, "%d", num);
    return string(s);
}

void Lib::sysErrLog(string logPath, string logName, CThostFtdcRspInfoField *info, int id, int isLast)
{
    if (info && info->ErrorID != 0) {
        ofstream sysLogger;
        logPath = logPath + getDate("%Y%m%d") + ".log";
        sysLogger.open(logPath.c_str(), ios::app);

        sysLogger << getDate("%Y%m%d-%H:%M:%S", true) << "|";
        sysLogger << "[ERROR]" << "|";
        sysLogger << logName << "|";
        sysLogger << "ErrCode" << "|" << info->ErrorID << "|";
        sysLogger << "ErrMsg" << "|" << info->ErrorMsg << "|";
        sysLogger << "RequestID" << "|" << id << "|";
        sysLogger << "IsLast" << "|" << isLast << endl;

        sysLogger.close();
    }
}

void Lib::sysReqLog(string logPath, string logName, int code)
{
    ofstream sysLogger;
    logPath = logPath + getDate("%Y%m%d") + ".log";
    sysLogger.open(logPath.c_str(), ios::app);

    sysLogger << getDate("%Y%m%d-%H:%M:%S", true) << "|";
    sysLogger << "[REQUEST]" << "|";
    sysLogger << logName << "|";
    sysLogger << "Code" << "|" << code << endl;

    sysLogger.close();
}


void Lib::initInfoLogHandle(string logPath, ofstream & infoHandle)
{
    logPath = logPath + getDate("%Y%m%d") + ".log";
    infoHandle.open(logPath.c_str(), ios::app);
    infoHandle << getDate("%Y%m%d-%H:%M:%S", true) << "|";
    infoHandle << "[INFO]" << "|";
}

void Lib::initMarketLogHandle(string logPath, ofstream & handle)
{
    logPath = logPath + "market_" + getDate("%Y%m%d") + ".log";
    handle.open(logPath.c_str(), ios::app);
    handle << getDate("%Y%m%d-%H:%M:%S", true) << "|";
}


vector<string> Lib::split(const string& s, const string& delim)
{
    vector<string> elems;
    size_t pos = 0;
    size_t len = s.length();
    size_t delim_len = delim.length();
    if (delim_len == 0) return elems;
    while (pos < len)
    {
        int find_pos = s.find(delim, pos);
        if (find_pos < 0)
        {
            elems.push_back(s.substr(pos, len - pos));
            break;
        }
        elems.push_back(s.substr(pos, find_pos - pos));
        pos = find_pos + delim_len;
    }
    return elems;
}

double Lib::max(double arr[], int cnt)
{
    double res = arr[0];
    for (int i = 0; i < cnt; ++i)
    {
        res = res > arr[i] ? res : arr[i];
    }
    return res;
}

double Lib::min(double arr[], int cnt)
{
    double res = arr[0];
    for (int i = 0; i < cnt; ++i)
    {
        res = res < arr[i] ? res : arr[i];
    }
    return res;
}

double Lib::mean(double arr[], int cnt)
{
    double sum = 0;
    for (int i = 0; i < cnt; ++i)
    {
        sum += arr[i];
    }
    return sum / cnt;
}

string Lib::tickData2String(TickData tick)
{
    string str = Lib::dtos(tick.price) + "_" +
                 Lib::itos(tick.volume) + "_" +
                 Lib::dtos(tick.bidPrice1) + "_" +
                 Lib::dtos(tick.askPrice1) + "_" +
                 string(tick.date) + "_" +
                 string(tick.time) + "_" +
                 Lib::itos(tick.msec) + "_" +
                 string(tick.instrumnetID);
    return str;
}

TickData Lib::string2TickData(string str)
{
    std::vector<string> params;
    params = split(str, "_");
    Tick tick = {0};
    tick.price = Lib::stod(params[0]);
    tick.volume = Lib::stoi(params[1]);
    tick.bidPrice1 = Lib::stod(params[2]);
    tick.askPrice1 = Lib::stod(params[3]);
    strcpy(tick.date, params[4].c_str());
    strcpy(tick.time, params[5].c_str());
    tick.msec = Lib::stoi(params[6]);
    strcpy(tick.instrumnetID, params[7].c_str());
    return tick;
}

