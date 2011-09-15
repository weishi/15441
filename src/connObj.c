#include "connObj.h"

int compareConnObj(void *data1, void *data2)
{
    return ((connObj *)data1)->connFd - ((connObj *)data2)->connFd;
}

void freeConnObj(void *data)
{
    connObj *connPtr = data;
    close(connPtr->connFd);
    char *buffer = (connPtr->buffer);
    free(buffer);
    free(connPtr);
}

int mapConnObj(void *data)
{
    return ((connObj *)data)->isOpen;
}


connObj *createConnObj(int connFd, ssize_t bufferSize)
{
    connObj *newObj = malloc(sizeof(connObj));
    newObj->connFd = connFd;
    newObj->curSize = 0;
    newObj->maxSize = bufferSize;
    newObj->isOpen = 1;
    newObj->buffer = (bufferSize > 0) ? malloc(bufferSize) : NULL;
    return newObj;
}


int getConnObjSocket(connObj *connPtr)
{
    return connPtr->connFd;
}

void getConnObjReadBuffer(connObj *connPtr, char **buf, ssize_t *size)
{
    int emptySize = connPtr->maxSize - connPtr->curSize;
    *buf = connPtr->buffer + connPtr->curSize;
    *size = emptySize;
}

void getConnObjWriteBuffer(connObj *connPtr, char **buf, ssize_t *size)
{
    *buf = connPtr->buffer;
    *size = connPtr->curSize;;
}

void setConnObjClose(connObj *connPtr)
{
    connPtr->isOpen = 0;
}

void setConnObjReadSize(connObj *connPtr, ssize_t readSize)
{
    connPtr->curSize += readSize;
}

void setConnObjWriteSize(connObj *connPtr, ssize_t writeSize)
{
    ssize_t curSize = connPtr->curSize;
    if(writeSize <= curSize) {
        char *buf = connPtr->buffer;
        memmove(buf, buf + writeSize, curSize - writeSize);
        connPtr->curSize -= writeSize;
    }
}

int isFullConnObj(connObj *connPtr)
{
    return connPtr->curSize == connPtr->maxSize;
}

int isEmptyConnObj(connObj *connPtr)
{
    return connPtr->curSize == 0;
}



