#ifndef SELECTENGINE_H
#define SELECTENGINE_H

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "httpHandler.h"
#include "linkedList.h"

typedef struct selectEngine {
    int port;
    char* logFile;
    int (*newConnHandler)(int);
    int (*closeConnHandler)(int);
} selectEngine;

void initEngine(selectEngine* engine,
                int port,
                char* logFile,
                int (*newConnHandler)(int),
                int (*closeConnHandler)(int) );

int startEngine(selectEngine*);
void listenSocket(selectEngine*, int listenFd);
int openSocket(int);
int closeSocket(int);

#endif
