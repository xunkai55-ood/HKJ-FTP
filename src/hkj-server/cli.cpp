#include <iostream>

#include "def.h"
#include "hkjserver.h"

int main(int argc, char *argv[]) {
    short port;
    if (argc < 2) {
        port = DEFAULT_CPORT;
    } else {
        port = (short)atol(argv[1]);
    }
    HKJServer server(port);
    server.mainLoop();
    return 0;
}