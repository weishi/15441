#ifndef SELECTENGINE_H
#define SELECTENGINE_H

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "connHandler.h"
#include "sslLib.h"

typedef struct selectEngine {
    int portHTTP;
    int portHTTPS;
    SSL_CTX *ctx;
    int  (*newConnHandler)(connObj *, char **);
    void (*readConnHandler)(connObj *);
    void (*processConnHandler)(connObj *);
    void (*writeConnHandler)(connObj *);
    int (*closeConnHandler)(connObj *);
} selectEngine;

void initEngine(selectEngine *engine,
                int portHTTP,
                int portHTTPS,
                int (*newConnHandler)(connObj *, char **),
                void (*readConnHandler)(connObj *),
                void (*processConnHandler)(connObj *),
                void (*writeConnHandler)(connObj *),
                int (*closeConnHandler)(connObj *),
                char *crtFile,
                char *keyFile
               );

int startEngine(selectEngine *);
int listenSocket(selectEngine *, int, int);
int openSocket(int);
int closeSocket(int);

void createPool(DLL *, fd_set *, fd_set *, int *);
void handlePool(DLL *, fd_set *, fd_set *, selectEngine *);



#endif
