#ifndef SOCKET_H
#define SOCKET_H

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;

int getSSocket(int port);

int getCSocket(const char * ip, int port);

void sendMsg(int cfd, string msg);

#endif
