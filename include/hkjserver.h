#ifndef HKJSERVER_H
#define HKJSERVER_H

#include <netinet/in.h>


class HKJServer {

public:
    HKJServer(short port);
    ~HKJServer();

    void mainLoop();
    static char helpInfo[];

private:
    short cPort_;
    int cSocket_;

    void processConnection(int id, int cSock, int dSock);
};


#endif