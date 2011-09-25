
#include "connHandler.h"

int newConnectionHandler(connObj *connPtr)
{
    struct sockaddr_in clientAddr;
    socklen_t clientLength = sizeof(clientAddr);
    int listenFd = getConnObjSocket(connPtr);
    int newFd = accept(listenFd, (struct sockaddr *)&clientAddr, &clientLength);
    if(newFd == -1) {
        fprintf(stderr, "Error accepting socket.\n");
        return -1;
    } else {
        return newFd;
    }
}

void processConnectionHandler(connObj *connPtr){
    char *buf;
    ssize_t size;
    getConnObjReadBuffer(connPtr, &buf, &size);
    switch(httpParse(connObj->req, buf, size)){
        case Parsing:
            return;
        case Parsed:
            
        case ParseError:
            setConnObjClose(connPtr);
        default:
    }
}



void readConnectionHandler(connObj *connPtr)
{
    if(!isFullConnObj(connPtr)) {
        char *buf;
        ssize_t size;
        int connFd = getConnObjSocket(connPtr);
        ssize_t readret = 0;
        getConnObjReadBuffer(connPtr, &buf, &size);

        readret = recv(connFd, buf, size, 0);
        if (readret == -1) {
            fprintf(stderr, "Error reading from client socket.\n");
            setConnObjClose(connPtr);
            return;
        } else if(readret == 0) {
            fprintf(stdout, "[%d] Close",connFd);
            setConnObjClose(connPtr);
            return;
        } else {
            setConnObjReadSize(connPtr,readret);
        }

    }

}


void writeConnectionHandler(connObj *connPtr)
{
    char *buf;
    ssize_t size;
    int connFd = getConnObjSocket(connPtr);
    getConnObjWriteBuffer(connPtr, &buf, &size);
    if (send(connFd, buf, size, 0) != size) {
        fprintf(stderr, "Error sending to client.\n");
        setConnObjClose(connPtr);
    } else {
        setConnObjWriteSize(connPtr, size);
    }

}


int closeConnectionHandler(connObj *connPtr)
{
    connPtr = connPtr;
    return EXIT_SUCCESS;
}
