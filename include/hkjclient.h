#ifndef HKJCLIENT_H
#define HKJCLIENT_H

class HKJClient {

public:
    HKJClient(char *serverIP, short serverPort);
    ~HKJClient();

    void mainLoop();

private:
    short cPort_, dPort_;
    int cSocket_, dSocket_;

};

#endif