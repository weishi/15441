#include "connHandler.h"

int newConnectionHandler(connObj *connPtr)
{
    struct sockaddr_in clientAddr;
    socklen_t clientLength = sizeof(clientAddr);
    int listenFd = getConnObjSocket(connPtr);
    int newFd = accept(listenFd, (struct sockaddr *)&clientAddr, &clientLength);
    if(newFd == -1) {
        printf("Error accepting socket.\n");
        return -1;
    } else {
        return newFd;
    }
}

void processConnectionHandler(connObj *connPtr)
{
    char *readBuf, *writeBuf;
    ssize_t rSize, wSize;
    int full, retVal;
    if(connPtr->isOpen == 0) {
        printf("Skip connection set to close\n");
        return;
    }
    switch(getConnObjType(connPtr)) {
    case TCP:
        getConnObjReadBufferForRead(connPtr, &readBuf, &rSize);
        getConnObjWriteBufferForWrite(connPtr, &writeBuf, &wSize);
        full = isFullConnObj(connPtr);
        retVal = flaskParse(readBuf, rSize, writeBuf, &wSize, full);
        if(retVal == -1) {
            setConnObjClose(connPtr);
        } else if(retVal == 0) {
            //Wait for next round.
        } else {
            addConnObjWriteSize(connPtr, wSize);
            setConnObjIsWrite(connPtr);
        }
        break;
    case UDP:
        break;
    default:
        break;
    }
    return;
}


void readConnectionHandler(connObj *connPtr)
{
    if(!isFullConnObj(connPtr)) {
        char *buf;
        ssize_t size;
        int connFd = getConnObjSocket(connPtr);
        ssize_t retSize = 0;
        getConnObjReadBufferForWrite(connPtr, &buf, &size);
        switch(getConnObjType(connPtr)) {
        case TCP:
            printf("Flask client...");
            retSize = recv(connFd, buf, size, 0);
            break;
        case UDP:
            printf("Peer routers...");
            retSize = -1;
            break;
        default:
            retSize = -1 ;
        }
        if (retSize == -1) {
            printf("Error reading from client.\n");
            if(errno == EINTR) {
                printf("RECV EINTR. Try later again.\n");
                return;
            }
            setConnObjClose(connPtr);
            return;
        } else if(retSize == 0) {
            printf("Client Closed [%d]", connFd);
            setConnObjClose(connPtr);
            return;
        } else {
            printf("Read %zu bytes\n", retSize);
            addConnObjReadSize(connPtr, retSize);
        }
    }
}


void writeConnectionHandler(connObj *connPtr)
{
    char *buf;
    ssize_t size, retSize;
    int connFd = getConnObjSocket(connPtr);
    getConnObjWriteBufferForRead(connPtr, &buf, &size);
    printf("Ready to write %zu bytes...", size);
    switch(getConnObjType(connPtr)) {
    case TCP:
        printf("Sending to Flask");
        retSize = send(connFd, buf, size, 0);
        break;
    case UDP:
        retSize = -1;
        break;
    default:
        retSize = -1;
    }
    if(retSize == -1) {
        if(errno == EINTR) {
            printf("RECV EINTR. Try later again.\n");
            return;
        } else {
            printf("Error sending to client. Close connection.\n");
            setConnObjClose(connPtr);
        }
    } else if(retSize != size) {
        printf("Sending to client with short count.\n");
        removeConnObjWriteSize(connPtr, retSize);
    } else {
        printf("WriteBuf cleared. Close connection.\n");
        removeConnObjWriteSize(connPtr, size);
        setConnObjClose(connPtr);
    }
}

