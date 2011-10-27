#ifndef ROUTINGTABLE_H
#define ROUTINGTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "linkedList.h"

/* Routing Entry for DLL */

typedef struct routingEntry {
    unsigned int nodeID;
    int isMe;
    char *host;
    int routingPort;
    int localPort;
    int serverPort;
    resourceTable *tRes;
} routingEntry;

int compareRoutingEntry(void *, void *);
void freeRoutingEntry(void *);

/* Routing Table */

typedef struct routingTable {
    DLL *table;
} routingTable;

int initRoutingTable(routingTable *, int nodeID, char *rouFile, char *resFile);

void getResourcePath(char*, char*, int*, char*);

/* Private methods */
int loadRoutingTable(routingTable *, char *);
routingEntry *parseRoutingLine(char *);
#endif
