#ifndef CONNOBJ_H
#define CONNOBJ_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "sslLib.h"
#include "httpResponder.h"

enum writeStatus {
    initRes,
    writingRes,
    lastRes,
    doneRes,
};

enum HTTPType {
    T_HTTP,
    T_HTTPS,
};


typedef struct connObj {
    int connFd;
    enum HTTPType connType;
    SSL *connSSL;
    int acceptedSSL;
    ssize_t curReadSize;
    ssize_t maxReadSize;
    ssize_t curWriteSize;
    ssize_t maxWriteSize;
    int isOpen;
    enum writeStatus wbStatus;
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
void getConnObjReadBufferForRead(connObj *, char **, ssize_t *);
void getConnObjReadBufferForWrite(connObj *, char **, ssize_t *);
void getConnObjWriteBufferForRead(connObj *, char **, ssize_t *);
void getConnObjWriteBufferForWrite(connObj *, char **, ssize_t *);

void setConnObjClose(connObj *);
void addConnObjReadSize(connObj *, ssize_t);
void removeConnObjReadSize(connObj *, ssize_t);
void addConnObjWriteSize(connObj *, ssize_t);
void removeConnObjWriteSize(connObj *, ssize_t);

void setConnObjHTTP(connObj *);
void setConnObjHTTPS(connObj *, SSL_CTX*);

int isHTTP(connObj *);
int isHTTPS(connObj *);

int hasAcceptedSSL(connObj *);
void setAcceptedSSL(connObj *);
int isFullConnObj(connObj *);
int isEmptyConnObj(connObj *);



#endif
