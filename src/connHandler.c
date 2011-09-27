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

void processConnectionHandler(connObj *connPtr)
{
    char *buf;
    ssize_t size, retSize;
    getConnObjReadBuffer(connPtr, &buf, &size);
    switch(httpParse(connPtr->req, buf, &size)) {
    case Parsing:
        removeConnObjReadSize(connPtr, size);
        break;
    case Parsed:
        removeConnObjReadSize(connPtr, size);
        if(connPtr == NULL) {
            connPtr->res = createResponseObj();
            buildResponseObj(connPtr->res, connPtr->req);
        }
        /* Dump response to buffer */
        getConnObjWriteBufferForWrite(connPtr, &buf, &size);
        retSize = writeResponse(connPtr->res, buf, size);
        if(retSize == 0) {
            if(1 == toClose(connPtr->res)) {
                setConnObjClose(connPtr);
            }
            /* Prepare for next request */
            freeResponseObj(connPtr->res);
            freeRequestObj(connPtr->req);
            connPtr->req = createRequestObj();
        } else {
            addConnObjWriteSize(connPtr, retSize);
        }

        break;
    case ParseError:
        setConnObjClose(connPtr);
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
        ssize_t readret = 0;
        getConnObjReadBuffer(connPtr, &buf, &size);

        readret = recv(connFd, buf, size, 0);
        if (readret == -1) {
            fprintf(stderr, "Error reading from client socket.\n");
            setConnObjClose(connPtr);
            return;
        } else if(readret == 0) {
            fprintf(stdout, "[%d] Close", connFd);
            setConnObjClose(connPtr);
            return;
        } else {
            addConnObjReadSize(connPtr, readret);
        }

    }

}


void writeConnectionHandler(connObj *connPtr)
{
    char *buf;
    ssize_t size;
    int connFd = getConnObjSocket(connPtr);
    getConnObjWriteBufferForRead(connPtr, &buf, &size);
    if (send(connFd, buf, size, 0) != size) {
        fprintf(stderr, "Error sending to client.\n");
        setConnObjClose(connPtr);
    } else {
        removeConnObjWriteSize(connPtr, size);
    }

}


int closeConnectionHandler(connObj *connPtr)
{
    connPtr = connPtr;
    return EXIT_SUCCESS;
}
