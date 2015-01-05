#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include "exception.h"
#include "def.h"
#include "hkjclient.h"

HKJClient::HKJClient(char *serverIP, short serverCPort) {
    cSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (cSocket_ < 0) {
        throw SocketFailed();
    }
    dSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (dSocket_ < 0) {
        throw SocketFailed();
    }

    sockaddr_in serverCAddr;
    memset(&serverCAddr, 0, sizeof(serverCAddr));
    serverCAddr.sin_family = AF_INET;
    inet_pton(AF_INET, serverIP, &(serverCAddr.sin_addr));
    serverCAddr.sin_port = htons(serverCPort);
    std::cout << serverCAddr.sin_addr.s_addr << std::endl;
    std::cout << serverCPort << std::endl;

    int k;
    if ((k = connect(cSocket_, (sockaddr *) &serverCAddr, sizeof(serverCAddr))) < 0) {
        std::cout << "[INFO]Cannot connect server ctrl socket" << std::endl;
        throw ConnectFailed();
    }

    short serverDPort;
    read(cSocket_, &serverDPort, sizeof(short));
    sockaddr_in serverDAddr;
    memset(&serverDAddr, 0, sizeof(serverDAddr));
    serverDAddr.sin_family = AF_INET;
    inet_pton(AF_INET, serverIP, &(serverDAddr.sin_addr));
    serverDAddr.sin_port = htons(serverDPort);

    if ((k = connect(dSocket_, (sockaddr *) &serverDAddr, sizeof(serverDAddr))) < 0) {
        std::cout << "[INFO]Cannot connect server data socket" << std::endl;
        throw ConnectFailed();
    }
    std::cout << "client started, data port" << ntohs(serverDPort) << std::endl;

}

HKJClient::~HKJClient() {
    close(cSocket_);
    close(dSocket_);

    std::cout << "client closed" << std::endl;
}

void HKJClient::mainLoop() {
    char buffer[BUFFER_SIZE];
    char cmd[COMMAND_SIZE];
    char tmp[BUFFER_SIZE];
    char op[COMMAND_SIZE];
    char fileName[COMMAND_SIZE];
    while (true) {
        memset(cmd, 0, sizeof(cmd));
        gets(cmd);
        memset(op, 0, sizeof(op));
        std::cout << "HKJ-CLIENT $ ";
        sscanf(cmd, "%s", op);
        if (strcmp(op, "?") == 0) {
            write(cSocket_, cmd, sizeof(cmd));
            memset(buffer, 0, sizeof(buffer));
            read(dSocket_, buffer, sizeof(buffer));
            std::cout << "[?]" << buffer << std::endl;
        } else if (strcmp(op, "pwd") == 0) {
            write(cSocket_, cmd, sizeof(cmd));
            memset(buffer, 0, sizeof(buffer));
            read(dSocket_, buffer, sizeof(buffer));
            std::cout << "[PWD]" << buffer << std::endl;
        } else if (strcmp(op, "cd") == 0) {
            write(cSocket_, cmd, sizeof(cmd));
            memset(buffer, 0, sizeof(buffer));
            read(dSocket_, buffer, sizeof(buffer));
            std::cout << "[CD]" << buffer << std::endl;
        } else if (strcmp(op, "dir") == 0) {
            write(cSocket_, cmd, sizeof(cmd));
            memset(buffer, 0, sizeof(buffer));
            read(dSocket_, buffer, sizeof(buffer));
            std::cout << "[DIR]" << buffer << std::endl;
        } else if (strcmp(op, "put") == 0) {
            memset(buffer, 0, BUFFER_SIZE);
            memset(tmp, 0, BUFFER_SIZE);
            getcwd(tmp, BUFFER_SIZE);
            sscanf(cmd, "%s %s", op, fileName);
            strcat(tmp, "/");
            strcat(tmp, fileName);
            FILE *f = fopen(fileName, "r");
            int n = -1;
            int total = 0;
            if (!f) {
                std::cout << "[PUT]" << fileName << " cannot be opened" << std::endl;
            } else {
                write(cSocket_, cmd, sizeof(cmd));
                while (true) {
                    n = (int)fread(buffer, 1, BUFFER_SIZE, f);
                    total += n;
                    write(cSocket_, &n, sizeof(n));
                    write(dSocket_, buffer, (size_t)n);
                    if (n < BUFFER_SIZE)
                        break;
                }
                std::cout << "[PUT]" << fileName << " done, size " << total / 1024 << "KB" << std::endl;
            }
        } else if (strcmp(op, "get") == 0) {
            sscanf(cmd, "%s %s", op, fileName);
            write(cSocket_, cmd, sizeof(cmd));
            write(dSocket_, fileName, strlen(fileName) + 1);
            FILE *f = fopen(fileName, "w");
            int n;
            int total = 0;
            while (true) {
                read(cSocket_, &n, sizeof(int));
                if (n <= 0) break;
                total += n;
                read(dSocket_, buffer, (size_t)n);
                fwrite(buffer, 1, (size_t)n, f);
                if (n < BUFFER_SIZE) {
                    n = 0;
                    fclose(f);
                    break;
                }
            }
            if (n == 0) {
                std::cout << "[GET]" << fileName << " done, size " << total / 1024 << "KB" << std::endl;
            } else {
                std::cout << "[GET]" << fileName << " failed, no such file" << std::endl;
            }
        } else if (strcmp(op, "quit") == 0) {
            memset(buffer, 0, sizeof(buffer));
            break;
        } else {
            // invalid operation
            std::cout << "Unknown operation" << std::endl;
        }
    }
}