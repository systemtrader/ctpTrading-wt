#ifndef MSG_H
#define MSG_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/msg.h>
#include <string>
#include <iostream>

using namespace std;

#define MAX_BUF 1024
typedef bool (*ACTIONCALLBACK)(long int, const void*);

class QService
{
private:
    ACTIONCALLBACK _callback;
    int _msgID;
    int _msgStructLen;
public:
    QService(int, int);
    ~QService();

    void setAction(ACTIONCALLBACK);
    void run();

};

class QClient
{
private:
    int _msgLen;
    int _msgID;
public:
    QClient(int, int);
    ~QClient();
    void send(void*);
};

#endif
