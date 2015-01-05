#include <iostream>

#include "def.h"
#include "hkjclient.h"

int main(int argc, char *argv[]) {
    short serverCPort;
    char *serverIP;
    char defaultIP[] = DEFAULT_IP;
    if (argc < 2) {
        serverIP = defaultIP;
    } else {
        serverIP = argv[1];
    }
    if (argc < 3) {
        serverCPort = DEFAULT_CPORT;
    } else {
        serverCPort = (short)atol(argv[2]);
    }
    HKJClient client(serverIP, serverCPort);
    client.mainLoop();
    return 0;
}