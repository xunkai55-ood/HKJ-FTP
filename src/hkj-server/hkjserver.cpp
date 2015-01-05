#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include "hkjserver.h"
#include "exception.h"
#include "def.h"

char HKJServer::helpInfo[] = "ops: get / put / dir / pwd / cd / ? / quit";

HKJServer::HKJServer(short port) {

    cPort_ = port;

    cSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (cSocket_ < 0) {
        throw SocketFailed();
    }

    sockaddr_in cAddr;
    memset(&cAddr, 0, sizeof(cAddr));
    cAddr.sin_family = AF_INET;
    cAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    cAddr.sin_port = htons(cPort_);

    int k = 1;
    setsockopt(cSocket_, SOL_SOCKET, SO_REUSEADDR, &k, sizeof(k));
    if (bind(cSocket_, (sockaddr *) &cAddr, sizeof(cAddr)) < 0) {
        std::cout << "[INFO]Cannot bind ctrl socket" << std::endl;
        throw BindFailed();
    }
    listen(cSocket_, 2);

    std::cout << "server started" << std::endl;
    std::cout << "[CONFIG]ctrl port " << cPort_ << std::endl;

}

HKJServer::~HKJServer() {

    close(cSocket_);
    std::cout << "server closed" << std::endl;

}

void HKJServer::mainLoop() {
    sockaddr_in clientCAddr, clientDAddr;
    socklen_t clientCAddrLen, clientDAddrLen;
    int cnt = 0, child = -1;
    while (true) {
        cnt++;
        std::cout << "server waiting..." << std::endl;
        int cSock = accept(cSocket_, (sockaddr *) &clientCAddr, &clientCAddrLen);
        if (cSock >= 0) {
            std::cout << "request coming, new client " << cnt << std::endl;
            sockaddr_in dAddr, dAddrIn;
            memset(&dAddr, 0, sizeof(dAddr));
            dAddr.sin_family = AF_INET;
            dAddr.sin_addr.s_addr = htonl(INADDR_ANY);
            dAddr.sin_port = htons(0);
            int k = 1;
            socklen_t len;
            int dSocket = socket(AF_INET, SOCK_STREAM, 0);
            setsockopt(dSocket, SOL_SOCKET, SO_REUSEADDR, &k, sizeof(k));
            if (bind(dSocket, (sockaddr *) &dAddr, sizeof(dAddr)) < 0) {
                std::cout << "[INFO]Cannot bind data socket" << std::endl;
                throw BindFailed();
            }
            listen(dSocket, 2);
            memset(&dAddrIn, 0, sizeof(dAddrIn));
            if (getsockname(dSocket, (sockaddr *)&dAddrIn, &len) < 0){
                std::cout << "[INFO]failed to get hostname" << std::endl;
                throw BindFailed();
            }
            std::cout << dAddrIn.sin_port << std::endl;
            short dPort = ntohs(dAddrIn.sin_port);
            write(cSock, &dPort, sizeof(short));
            std::cout << "[CONFIG] port " << ntohs(dPort) << " for client " << cnt << std::endl;
            int dSock = accept(dSocket, (sockaddr *) &clientDAddr, &clientDAddrLen);
            child = fork();
            if (child == 0)
                processConnection(cnt, cSock, dSock);
            else if (child == -1)
                throw ForkFailed();
        }
    }
}

void HKJServer::processConnection(int id, int cSock, int dSock) {
    char cmd[COMMAND_SIZE];
    char op[COMMAND_SIZE];
    char tmp[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    std::cout << "PROCESSING CLIENT ID (" << id << ")" << std::endl;
    while (true) {
        memset(cmd, 0, sizeof(cmd));
        int readCnt = (int)read(cSock, cmd, sizeof(cmd));
        if (readCnt < 0) {
            throw ReadFailed();
        }
        if (readCnt == 0) {
            std::cout << "(" << id << ")[EMPTY COMMAND]" << cmd << std::endl;
            break;
        }
        std::cout << "(" << id << ")[COMMAND]" << cmd << std::endl;
        memset(op, 0, sizeof(op));
        sscanf(cmd, "%s", op);
        if (strcmp(op, "?") == 0) {
            write(dSock, helpInfo, sizeof(helpInfo));
        } else if (strcmp(op, "quit") == 0) {
            break;
        } else if (strcmp(op, "pwd") == 0) {
            memset(buffer, 0, BUFFER_SIZE);
            getcwd(buffer, BUFFER_SIZE);
            write(dSock, buffer, strlen(buffer) + 1);
        } else if (strcmp(op, "cd") == 0) {
            memset(buffer, 0, BUFFER_SIZE);
            sscanf(cmd, "%s %s", op, tmp);
            int res = chdir(tmp);
            if (res == 0) {
                strcpy(buffer, "ok");
            } else {
                strcpy(buffer, "cd failed");
            }
            write(dSock, buffer, strlen(buffer) + 1);
        } else if (strcmp(op, "dir") == 0) {
            memset(buffer, 0, BUFFER_SIZE);
            memset(tmp, 0, BUFFER_SIZE);
            getcwd(tmp, BUFFER_SIZE);
            DIR *d = opendir(tmp);
            dirent *entry;
            char *cursor = buffer;
            char fileType;
            while ((entry = readdir(d)) != 0) {
                memset(tmp, 0, sizeof(tmp));
                if (entry->d_type == DT_DIR)
                    fileType = 'D';
                else if (entry->d_type == DT_REG)
                    fileType = 'F';
                else
                    fileType = 'U';
                sprintf(tmp, "[%c]%s\t", fileType, entry->d_name);
                if (cursor + strlen(tmp) >= buffer + sizeof(buffer)) {
                    break;
                }
                size_t n = strlen(tmp);
                memcpy(cursor, tmp, n);
                cursor += n;
            }
            *cursor = '\0';
            write(dSock, buffer, strlen(buffer) + 1);
            closedir(d);
        } else if (strcmp(op, "get") == 0) {
            memset(buffer, 0, BUFFER_SIZE);
            memset(tmp, 0, BUFFER_SIZE);
            getcwd(tmp, BUFFER_SIZE);
            sscanf(cmd, "%s %s", op, buffer);
            strcat(tmp, "/");
            strcat(tmp, buffer);
            FILE *f = fopen(buffer, "r");
            int n = -1;
            if (!f) {
                write(cSock, &n, sizeof(n));
            } else {
                while (true) {
                    n = (int) fread(buffer, 1, BUFFER_SIZE, f);
                    write(cSock, &n, sizeof(n));
                    write(dSock, buffer, (size_t) n);
                    if (n < BUFFER_SIZE)
                        break;
                }
            }
        } else if (strcmp(op, "put") == 0) {
            memset(tmp, 0, sizeof(tmp));
            sscanf(cmd, "%s %s", op, tmp);
            std::cout << tmp << std::endl;
            char *t = tmp + strlen(tmp);
            while (t != tmp && *t != '/') t--;
            if (*t == '/') t++;
            std::cout << "Writing to " << t << " ..." << std::endl;
            FILE *f = fopen(t, "w");
            int n;
            while (true) {
                read(cSock, &n, sizeof(int));
                if (n <= 0) break;
                read(dSock, buffer, (size_t)n);
                fwrite(buffer, 1, (size_t)n, f);
                if (n < BUFFER_SIZE) {
                    n = 0;
                    fclose(f);
                    break;
                }
            }
        } else {
            std::cout << "(" << id << ")[INFO]Unknown operation" << std::endl;
                // invalid operation, do nothing
        }
    }
    std::cout << "(" << id << ")[QUIT]" << std::endl;
    exit(0);
}

