#ifndef CONNOBJ_H
#define CONNOBJ_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

enum connType {
    TCP,
    UDP
};


typedef struct connObj {
    int connFd;
    ssize_t curReadSize;
    ssize_t maxReadSize;
    ssize_t curWriteSize;
    ssize_t maxWriteSize;
    int isOpen;
    int isRead;
    int isWrite;
    char *readBuffer;
    char *writeBuffer;
    DLL *LSAList;
    enum connType type;
} connObj;



int compareConnObj(void *data1, void *data2);
void freeConnObj(void *data);
int mapConnObj(void *data);

/* Constructor */
connObj *createConnObj(int, ssize_t, enum connType);

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

int isReadConnObj(connObj *);
int isWriteConnObj(connObj *);

int isFullConnObj(connObj *);
int isEmptyConnObj(connObj *);

enum connType getConnObjType(connObj *);

void setConnObjIsWrite(connObj *);
#endif
