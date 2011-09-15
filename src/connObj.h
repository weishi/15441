#ifndef CONNOBJ_H
#define CONNOBJ_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>


typedef struct connObj {
    int connFd;
    ssize_t curSize;
    ssize_t maxSize;
    int isOpen;
    char *buffer;
} connObj;


int compareConnObj(void *data1, void *data2);
void freeConnObj(void *data);
int mapConnObj(void *data);

/* Constructor */
connObj *createConnObj();

/* Getters and Setters */
int getConnObjSocket(connObj *);
void getConnObjReadBuffer(connObj *, char **, ssize_t *);
void getConnObjWriteBuffer(connObj *, char **, ssize_t *);

void setConnObjClose(connObj *);
void setConnObjReadSize(connObj *, ssize_t);
void setConnObjWriteSize(connObj *, ssize_t);

int isFullConnObj(connObj *);
int isEmptyConnObj(connObj *);



#endif
