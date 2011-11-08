#ifndef ROUTINGTABLE_H
#define ROUTINGTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "linkedList.h"
#include "resourceTable.h"


typedef struct routingInfo {
    const char *host;
    int port;
    const char *path;
} routingInfo;

/* Routing Entry for DLL */

typedef struct routingEntry {
    unsigned int nodeID;
    int isMe;
    char *host;
    int routingPort;
    int localPort;
    int serverPort;
    resourceTable *tRes;
    LSA *lsaPacket;
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

int getRoutingInfo(char *, routingInfo *);
int getRoutingPort(unsigned int);
int getLocalPort(unsigned int);

void insertLocalResource(char *, char*);
void updateRoutingTableFromLSA(LSA *);

/* Private methods */
int loadRoutingTable(routingTable *, unsigned int nodeID, char *, char *);
routingEntry *parseRoutingLine(char *);
routingEntry *getRoutingEntry(unsigned int);
routingEntry *getMyRoutingEntry();
#endif
