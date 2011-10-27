#ifndef ROUTINGENGINE_H
#define ROUTINGENGINE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "routingTable.h"
#include "resourceTable.h"
#include "linkedList.h"

typedef struct routingEngine {
    routingTable *tRou;
    resourceTable *tRes;
    int nodeID;
    int cycleTime;
    int neighborTimeout;
    int retranTimeout;
    int LSATimeout;
} routingEngine;

/* Public methods */
void initRouter(routingEngine *router,
                routingTable *tRouting,
                resourceTable *tResource,
                int nodeID,
                int cycleTime,
                int neighborTimeout,
                int retranTimeout,
                int LSATimeout);

int startRouter(routingEngine *);

/* Private methods */
void exitRouter(routingEngine *, DLL*);
int listenSocket(routingEngine *, int, int);
int openSocket(int);
int closeSocket(int);
void signalExitRouter();
void signalRestartRouter();

void createPool(DLL *, fd_set *, fd_set *, int *);
void handlePool(DLL *, fd_set *, fd_set *, routingEngine *);
#endif
