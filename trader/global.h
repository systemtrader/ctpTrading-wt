#ifndef GLOBAL_H
#define GLOBAL_H

#include "../ThostFtdcUserApiStruct.h"
#include <string>
using namespace std;

static string brokerID;
static string userID;
static string password;

static int reqID = 0;
static int orderRef = 0;

///前置编号
static TThostFtdcFrontIDType   frontID;
///会话编号
static TThostFtdcSessionIDType sessionID;

#endif