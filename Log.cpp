#include "log.h"

Logger::Logger(const string & path)
{
    _fileHandle.open(path.c_str(), ios::app);
}

Logger::~Logger()
{
    _fileHandle.close();
}

void Logger::write(const string & info)
{
    _fileHandle << info << endl;
}
