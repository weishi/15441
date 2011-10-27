#include "connHandler.h"

int newConnectionHandler(connObj *connPtr, char **addr)
{
    struct sockaddr_in clientAddr;
    socklen_t clientLength = sizeof(clientAddr);
    int listenFd = getConnObjSocket(connPtr);
    int newFd = accept(listenFd, (struct sockaddr *)&clientAddr, &clientLength);
    if(newFd == -1) {
        logger(LogProd, "Error accepting socket.\n");
        return -1;
    } else {
        *addr = inet_ntoa(clientAddr.sin_addr);
        return newFd;
    }
}

void processConnectionHandler(connObj *connPtr)
{
    char *buf;
    ssize_t size, retSize;
    int done, full;
    if(connPtr->isOpen == 0) {
        printf("Skip connection set to close\n");
        return;
    }
    getConnObjReadBufferForRead(connPtr, &buf, &size);
    full = isFullConnObj(connPtr);
    switch(httpParse(connPtr->req, buf, &size, full)) {
    case Parsing:
        removeConnObjReadSize(connPtr, size);
        break;
    case ParseError:
    case Parsed:
        removeConnObjReadSize(connPtr, size);
        if(connPtr->res == NULL) {
            printf("Create new response\n");
            connPtr->res = createResponseObj();
            buildResponseObj(connPtr->res, connPtr->req);
            if(isCGIResponse(connPtr->res)) {
                connPtr->CGIout = connPtr->res->CGIout;
            }
        }
        if(!isCGIResponse(connPtr->res)) {
            /* Dump HTTP response to buffer */
            printf("Dump response to buffer\n");
            getConnObjWriteBufferForWrite(connPtr, &buf, &size);
            printf("Write buffer has %d bytes free\n", size);
            done = writeResponse(connPtr->res, buf, size, &retSize);
            printf("%d bytes dumped, done? %d\n", retSize, done);
            addConnObjWriteSize(connPtr, retSize);
            connPtr->wbStatus = writingRes;
            if(done) {
                printf("All dumped\n");
                connPtr->wbStatus = lastRes;
            }
        }
        printf("Return from httpParse\n");
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
            logger(LogProd, "Error reading from client.\n");
            if(errno == EINTR) {
                logger(LogProd, "RECV EINTR. Try later again.\n");
                return;
            }
            setConnObjClose(connPtr);
            return;
        } else if(retSize == 0) {
            printf("Client Closed [%d]", connFd);
            setConnObjClose(connPtr);
            return;
        } else {
            printf("Read %d bytes\n", retSize);
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
    printf("Ready to write %d bytes...", size);
    if(size <= 0) {
        prepareNewConn(connPtr);
        return;
    }
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
    if(retSize == -1 && errno == EINTR) {
        return ;
    }
    if (retSize != size) {
        logger(LogProd, "Error sending to client.\n");
        setConnObjClose(connPtr);
    } else {
        printf("Done\n");
        removeConnObjWriteSize(connPtr, size);
    }

}

