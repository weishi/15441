#include "connObj.h"

int compareConnObj(void *data1, void *data2)
{
    return ((connObj *)data1)->connFd - ((connObj *)data2)->connFd;
}

void freeConnObj(void *data)
{
    connObj *connPtr = data;
    close(connPtr->connFd);
    free(connPtr->readBuffer);
    free(connPtr->writeBuffer);
    free(connPtr);
}

int mapConnObj(void *data)
{
    return ((connObj *)data)->isOpen;
}


connObj *createConnObj(int connFd,
                       ssize_t bufferSize,enum connType type)
{
    connObj *newObj = malloc(sizeof(connObj));
    newObj->connFd = connFd;
    newObj->type = type;
    newObj->curReadSize = 0;
    newObj->maxReadSize = bufferSize;
    newObj->curWriteSize = 0;
    newObj->maxWriteSize = bufferSize;
    newObj->isOpen = 1;
    newObj->readBuffer = (bufferSize > 0) ? malloc(bufferSize) : NULL;
    newObj->writeBuffer = (bufferSize > 0) ? malloc(bufferSize) : NULL;
    newObj->isRead=1;
    newObj->isWrite=1;
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
        return connPtr->curWriteSize == 0;
}

int isReadConnObj(connObj *connPtr)
{
    return connPtr->isRead == 1;
}

int isWriteConnObj(connObj *connPtr)
{
    return connPtr->isWrite == 1;
}

enum connType getConnObjType(connObj *connPtr){
    return connPtr->type;
}

void setConnObjIsWrite(connObj *connPtr){
    connPtr->isWrite=1;
    connPtr->isRead=0;
}
