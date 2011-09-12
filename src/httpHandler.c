
#include "httpHandler.h"

int newConnectionHandler(int listenFd)
{
    struct sockaddr_in clientAddr;
    socklen_t clientLength = sizeof(clientAddr);
    int newFd = accept(listenFd, (struct sockaddr *)&clientAddr, &clientLength);
    if(newFd == -1) {
        fprintf(stderr, "Error accepting socket.\n");
        return CLOSE_ME;
    } else {
        fprintf(stderr, "New Connection [%d]\n", newFd);
        return newFd;
    }
}

int oldConnectionHandler(int connFd)
{
    ssize_t readret = 0;
    char buf[BUF_SIZE];

    while((readret = recv(connFd,buf, BUF_SIZE, 0)) > 1) {
        if (send(connFd, buf, readret, 0) != readret) {
            fprintf(stderr, "Error sending to client.\n");
            return CLOSE_ME;
        }
        memset(buf, 0, BUF_SIZE);
    }

    if (readret == -1) {
        fprintf(stderr, "Error reading from client socket.\n");
        return CLOSE_ME;
    }
    fprintf(stderr, "Old Connection [%d]\n", connFd);

    return EXIT_SUCCESS;
}
int closeConnectionHandler(int closeFd)
{
    closeFd=closeFd;
    return EXIT_SUCCESS;
}
