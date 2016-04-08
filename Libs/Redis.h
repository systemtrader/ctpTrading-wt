#include <hiredis/hiredis.h>
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

class Redis
{
private:
    redisContext *pRedisContext;
    redisReply *pRedisReply;
public:
    Redis(string host, int port, int db);
    ~Redis();
    string execCmd(string cmd);
};
