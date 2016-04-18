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

#define MAX_BUF 1024
typedef void (*ACTIONCALLBACK)(long int, void*);

class QService
{
private:
    int id;
    ACTIONCALLBACK callback;
public:
    QService(int);
    ~QService();

    setAction(ACTIONCALLBACK);
    run();

};

class QClient
{
private:
    int id;
    int msgLen;
public:
    QClient(int, int);
    ~QClient();
    send(void*);
};

#endif
