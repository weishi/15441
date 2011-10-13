#include "connObj.h"

int compareConnObj(void *data1, void *data2)
{
    return ((connObj *)data1)->connFd - ((connObj *)data2)->connFd;
}

void freeConnObj(void *data)
{
    connObj *connPtr = data;
    if(isHTTPS(connPtr)) {
        SSL_shutdown(connPtr->connSSL);
        SSL_free(connPtr->connSSL);
    }
    close(connPtr->connFd);
    free(connPtr->clientAddr);
    free(connPtr->readBuffer);
    free(connPtr->writeBuffer);
    freeRequestObj(connPtr->req);
    connPtr->req = NULL;
    freeResponseObj(connPtr->res);
    connPtr->res = NULL;
    free(connPtr);
}

int mapConnObj(void *data)
{
    return ((connObj *)data)->isOpen;
}


connObj *createConnObj(int connFd, ssize_t bufferSize, int port, char *addr)
{
    connObj *newObj = malloc(sizeof(connObj));
    newObj->connFd = connFd;
    newObj->serverPort = port;
    newObj->clientAddr = NULL;
    if(addr != NULL) {
        char *buf = malloc(strlen(addr) + 1);
        strcpy(buf, addr);
        newObj->clientAddr = buf;
    }
    newObj->acceptedSSL = 0;
    newObj->curReadSize = 0;
    newObj->maxReadSize = bufferSize;
    newObj->curWriteSize = 0;
    newObj->maxWriteSize = bufferSize;
    newObj->isOpen = 1;
    newObj->wbStatus = initRes;
    newObj->readBuffer = (bufferSize > 0) ? malloc(bufferSize) : NULL;
    newObj->writeBuffer = (bufferSize > 0) ? malloc(bufferSize) : NULL;

    newObj->req = createRequestObj(newObj->serverPort, newObj->clientAddr);
    newObj->res = NULL;

    newObj->CGIout = -1;
    return newObj;
}


int getConnObjSocket(connObj *connPtr)
{
    return connPtr->connFd;
}

void getConnObjReadBufferForRead(connObj *connPtr, char **buf, ssize_t *size)
{
    *buf = connPtr->readBuffer;
    *size = connPtr->curReadSize;;
}

void getConnObjReadBufferForWrite(connObj *connPtr, char **buf, ssize_t *size)
{
    int emptySize = connPtr->maxReadSize - connPtr->curReadSize;
    *buf = connPtr->readBuffer + connPtr->curReadSize;
    *size = emptySize;
}

void getConnObjWriteBufferForRead(connObj *connPtr, char **buf, ssize_t *size)
{
    *buf = connPtr->writeBuffer;
    *size = connPtr->curWriteSize;;
}

void getConnObjWriteBufferForWrite(connObj *connPtr, char **buf, ssize_t *size)
{
    *buf = connPtr->writeBuffer + connPtr->curWriteSize;
    *size = connPtr->maxWriteSize - connPtr->curWriteSize;
}

void setConnObjClose(connObj *connPtr)
{
    connPtr->isOpen = 0;
}

void addConnObjReadSize(connObj *connPtr, ssize_t readSize)
{
    connPtr->curReadSize += readSize;
}

void addConnObjWriteSize(connObj *connPtr, ssize_t writeSize)
{
    connPtr->curWriteSize += writeSize;
}

void removeConnObjReadSize(connObj *connPtr, ssize_t readSize)
{
    ssize_t curReadSize = connPtr->curReadSize;
    if(readSize <= curReadSize) {
        char *buf = connPtr->readBuffer;
        memmove(buf, buf + readSize, curReadSize - readSize);
        connPtr->curReadSize -= readSize;
    }
}

void removeConnObjWriteSize(connObj *connPtr, ssize_t writeSize)
{
    ssize_t curWriteSize = connPtr->curWriteSize;
    if(writeSize <= curWriteSize) {
        char *buf = connPtr->writeBuffer;
        memmove(buf, buf + writeSize, curWriteSize - writeSize);
        connPtr->curWriteSize -= writeSize;
    }
}

int isFullConnObj(connObj *connPtr)
{
    return connPtr->curReadSize == connPtr->maxReadSize;
}

int isEmptyConnObj(connObj *connPtr)
{
    switch(connPtr->wbStatus) {
    case initRes:
        return connPtr->curWriteSize == 0;
    case writingRes:
    case lastRes:
        return 0;
    case doneRes:
        return 1;
    default:
        return -1;
    }
}

void setConnObjHTTP(connObj *connPtr)
{
    connPtr->connType = T_HTTP;
    connPtr->connSSL = NULL;
}

void setConnObjHTTPS(connObj *connPtr, SSL_CTX *ctx)
{
    /* Set non-blocking */
    int flag = fcntl(connPtr->connFd, F_GETFL, 0);
    flag = flag | O_NONBLOCK;
    fcntl(connPtr->connFd, F_SETFL, flag);
    /* Init SSL connection */
    SSL *sslPtr = SSL_new(ctx);
    SSL_set_fd(sslPtr, connPtr->connFd);
    connPtr->connSSL = sslPtr;
    connPtr->connType = T_HTTPS;
}

int isHTTP(connObj *connPtr)
{
    return connPtr->connType == T_HTTP;
}

int isHTTPS(connObj *connPtr)
{
    return connPtr->connType == T_HTTPS;
}

int hasAcceptedSSL(connObj *connPtr)
{
    return connPtr->acceptedSSL == 1;
}

void setAcceptedSSL(connObj *connPtr)
{
    connPtr->acceptedSSL = 1;
}


void cleanConnObjCGI(connObj *connPtr)
{
    close(connPtr->CGIout);
    connPtr->CGIout = -1;
    connPtr->wbStatus = lastRes;
};
