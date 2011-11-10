#ifndef ROUTINGTABLE_H
#define ROUTINGTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/time.h>

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
    LSA *lastLSA;
    int distance;
} routingEntry;

int compareRoutingEntry(void *, void *);
void freeRoutingEntry(void *);

/* Routing Table */
typedef struct routingTable {
    DLL *table;
    timeval *oldTime;
    DLL *LSAList;
    int cycleTime;
    int neighborTimeout;
    int retranTimeout;
    int LSATimeout;
} routingTable;

routingTable *tRouting;

int initRoutingTable(int nodeID,
                     char *rouFile, char *resFile,
                     int cycleTime,
                     int neighborTimeout,
                     int retranTimeout,
                     int LSATimeout);

int getRoutingInfo(char *, routingInfo *);
int getRoutingPort(unsigned int);
int getLocalPort(unsigned int);

void insertLocalResource(char *, char *);

/* Called from connHandler */
void updateRoutingTableFromLSA(LSA *);
void getLSAFromRoutingTable(DLL *);


/* Private methods */
int loadRoutingTable(routingTable *, unsigned int nodeID, char *, char *);
routingEntry *parseRoutingLine(char *);
routingEntry *getRoutingEntry(unsigned int);
void removeRoutingEntry(unsigned int);
routingEntry *getRoutingEntryByHost(char *);
routingEntry *getMyRoutingEntry();
unsigned int getLastNodeID(LSA *);
void updateTime();
void newAdvertisement(DLL *);
void updateLSArouting(LSA *);
void addLSAWithDest(DLL *, LSA *, unsigned int ignore);
void addLSAWithOneDest(DLL *, LSA *, unsigned int destID);

void addLSAtoBuffer(LSA *);

#endif
