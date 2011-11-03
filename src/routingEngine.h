#ifndef ROUTINGENGINE_H
#define ROUTINGENGINE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "connHandler.h"

typedef struct routingEngine {
    int nodeID;
    int cycleTime;
    int neighborTimeout;
    int retranTimeout;
    int LSATimeout;
    int (*newConnHandler)(connObj *); 
    void (*readConnHandler)(connObj *);
    void (*processConnHandler)(connObj *);
    void (*writeConnHandler)(connObj *);
} routingEngine;

/* Public methods */
void initRouter(routingEngine *router,
                int nodeID,
                int cycleTime,
                int neighborTimeout,
                int retranTimeout,
                int LSATimeout);

int startRouter(routingEngine *);

/* Private methods */
void exitRouter(routingEngine *, DLL *);
int listenSocket(routingEngine *, int, int);
int openSocket(int, char *);
int closeSocket(int);
void signalExitRouter();
void signalRestartRouter();

void createPool(DLL *, fd_set *, fd_set *, int *);
void handlePool(DLL *, fd_set *, fd_set *, routingEngine *);
#endif
