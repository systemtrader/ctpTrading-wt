#ifndef LOG_H
#define LOG_H

#include <fstream>
#include <string>

class Logger
{
private:
    ofstream _fileHandle;
public:
    void write(const string & info);
    Logger(const string & path);
    ~Logger();
};

#endif