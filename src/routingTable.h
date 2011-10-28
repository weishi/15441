#ifndef ROUTINGTABLE_H
#define ROUTINGTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limit.h>

#include "linkedList.h"
#include "resourceTable.h"


typedef struct routingInfo{
    const char *host;
    const int port;
    const char *path;
}routingInfo;

/* Routing Entry for DLL */

typedef struct routingEntry {
    unsigned int nodeID;
    int isMe;
    char *host;
    int routingPort;
    int localPort;
    int serverPort;
    resourceTable *tRes;
    int distance;
} routingEntry;

int compareRoutingEntry(void *, void *);
void freeRoutingEntry(void *);

/* Routing Table */


typedef struct routingTable {
    DLL *table;
} routingTable;

routingTable *tRouting;

int initRoutingTable(int nodeID, char *rouFile, char *resFile);

void getRoutingInfo(char *objName, routingInfo *rInfo);

/* Private methods */
int loadRoutingTable(routingTable *, char *);
routingEntry *parseRoutingLine(char *);
#endif
