#ifndef CONNOBJ_H
#define CONNOBJ_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "httpParser.h"
#include "httpResponder.h"

typedef struct connObj {
    int connFd;
    ssize_t curReadSize;
    ssize_t maxReadSize;
    ssize_t curWriteSize;
    ssize_t maxWriteSize;
    int isOpen;
    char *readBuffer;
    char *writeBuffer;
    requestObj *req;
    responseObj *res;
} connObj;


int compareConnObj(void *data1, void *data2);
void freeConnObj(void *data);
int mapConnObj(void *data);

/* Constructor */
connObj *createConnObj();

/* Getters and Setters */
int getConnObjSocket(connObj *);
void getConnObjReadBuffer(connObj *, char **, ssize_t *);
void getConnObjWriteBufferForRead(connObj *, char **, ssize_t *);
void getConnObjWriteBufferForWrite(connObj *, char **, ssize_t *);

void setConnObjClose(connObj *);
void addConnObjReadSize(connObj *, ssize_t);
void removeConnObjReadSize(connObj *, ssize_t);
void addConnObjWriteSize(connObj *, ssize_t);
void removeConnObjWriteSize(connObj *, ssize_t);

int isFullConnObj(connObj *);
int isEmptyConnObj(connObj *);



#endif
