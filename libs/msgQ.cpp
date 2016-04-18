#include "msgQ.h"

QService::QService(int qid, int msgStructLen)
{
    _msgStructLen = msgStructLen;

    //建立消息队列
    _msgID = msgget((key_t)qid, 0666 | IPC_CREAT);
    if(_msgID == -1)
    {
        fprintf(stderr, "msgget failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

QService::~QService()
{
    //删除消息队列
    if(msgctl(_msgID, IPC_RMID, 0) == -1)
    {
        fprintf(stderr, "msgctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }
}

void QService::setAction(ACTIONCALLBACK callback)
{
    _callback = callback;
}

void QService::run()
{
    void * data;
    data = malloc(_msgStructLen);

    while(true)
    {
        if(msgrcv(_msgID, data, MAX_BUF, 0, 0) == -1)
        {
            fprintf(stderr, "msgrcv failed with errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }
        long int msgType = *((long int *)data);
        if(!_callback(msgType, data)) break;
    }
    free(data);
}

QClient::QClient(int qid, int msgLen)
{
    _msgLen = msgLen;
    _msgID = msgget((key_t)qid, 0666 | IPC_CREAT);
    if(_msgID == -1)
    {
        fprintf(stderr, "msgget failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

QClient::~QClient()
{

}

void QClient::send(void * data)
{
    if(msgsnd(_msgID, data, _msgLen, 0) == -1)
    {
        fprintf(stderr, "msgsnd failed: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}