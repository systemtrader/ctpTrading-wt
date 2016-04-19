#include "Socket.h"

int getSSocket(int port)
{
    int sfd;
    struct sockaddr_in serverAddr;

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        cout << "create socket error: " << strerror(errno) << " params: " << port << endl;
        exit(0);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    if (bind(sfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        cout << "bind socket error: " << strerror(errno) << " params: " << port << endl;
        exit(0);
    }

    if (listen(sfd, 10) == -1) {
        cout << "listen socket error: " << strerror(errno) << " params: " << port << endl;
        exit(0);
    }
    return sfd;
}

int getCSocket(const char * ip, int port)
{
    int cfd;
    struct sockaddr_in serverAddr;

    if ((cfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "create socket error: " << strerror(errno) << " params: " << ip << ", " << port << endl;
        exit(0);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
        cout << "inet_pton error: " << strerror(errno) << " params: " << ip << ", " << port << endl;
        exit(0);
    }

    if (connect(cfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        cout << "connect error: " << strerror(errno) << " params: " << ip << ", " << port<< endl;
        exit(0);
    }
    return cfd;
}

void sendMsg(int cfd, string msg)
{
    if (send(cfd, msg.c_str(), strlen(msg.c_str()), 0) < 0) {
        cout << "send error: " << strerror(errno) << endl;
    }
}

