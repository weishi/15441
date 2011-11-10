#include "routingTable.h"



int compareRoutingEntry(void *data1, void *data2)
{
    routingEntry *re1 = (routingEntry *)data1;
    routingEntry *re2 = (routingEntry *)data2;
    return (re1->nodeID) - (re2->nodeID);
}

void freeRoutingEntry(void *data)
{
    routingEntry *re = (routingEntry *)data;
    free(re->host);
}

void newAdvertisement(DLL *list)
{
    LSA *newAds = NULL;
    timeval curTime;
    timeval *oldTime = tRouting->oldTime;
    gettimeofday(&curTime, NULL);
    routingEntry *myEntry = getMyRoutingEntry();
    if((myEntry->lastLSA == NULL)  ||
            (curTime.tv_sec - oldTime->tv_sec > tRouting->cycleTime)) {
        newAds = headerLSAFromLSA(myEntry->lastLSA);
        updateLSArouting(newAds);
        updateLSAresource(newAds, myEntry->tRes);
        updateTime();
        replaceLSA(&(myEntry->lastLSA), newAds);
        addLSAWithDest(list, newAds, 0);
    } else {
        return NULL;
    }
}

void addLSAWithDest(DLL *list, LSA *lsa, unsigned int ignoreID)
{
    DLL *table = tRouting->table;
    int i = 0;
    while(i < table->size) {
        routingEntry *entry = getNodeDataAt(table, i);
        if(entry->distance == 1 && entry->nodeID != ignoreID) {
            LSA *newLSA = LSAfromLSA(lsa);
            setLSADest(newLSA, entry->host, entry->routingPort);
            insertNode(list, newLSA);
        }
        i++;
    }
}
void addLSAWithOneDest(DLL *list, LSA *lsa, unsigned int destID)
{
    routingEntry *entry = getRoutingEntry(destID);
    setLSADest(lsa, entry->host, entry->routingPort);
    insertNode(list, lsa);
}

void updateLSArouting(LSA *lsa)
{
    DLL *table = tRouting->table;
    int i = 0;
    while(i < table->size) {
        routingEntry *entry = getNodeDataAt(table, i);
        if(entry->distance == 1) {
            insertLSAlink(lsa, entry->nodeID);
        }
        i++;
    }
}

/* Called from connHandler */
void updateRoutingTableFromLSA(LSA *lsa)
{
    int isNew, isHigher, isSame, isLower, isZero, isMine;
    unsigned int nodeID = lsa->senderID;
    if(isLSAAck(lsa)) {
        //Update hasACK
        routingEntry *senderEntry = getRoutingEntryByHost(lsa->src);
        if(!hasLSAACK(senderEntry->lastLSA)){
            gotLSAACK(senderEntry->lastLSA);
            printf("LSA ACK checked in.\n");
        }else{
            printf("Got ACK for something we didn't send...\n");
        }
    } else {
        routingEntry *entry = getRoutingEntry(nodeID);
        routingEntry *myEntry = getMyRoutingEntry();
        isZero = (lsa->TTL == 0);
        isNew = (entry == NULL && !isZero);
        isHigher = (entry != NULL && entry->lastLSA->seqNo < lsa->seqNo && !isZero);
        isSame = (entry != NULL && entry->lastLSA->seqNo == lsa->seqNo && !isZero);
        isLower = (entry != NULL && entry->lastLSA->seqNo > lsa->seqNo && !isZero);
        isMine = (entry != NULL &&  entry == myEntry);
        //Send back ACK (ANY)
        LSA *ack = headerLSAfromLSA(lsa);
        setLSAAck(ack);
        routingEntry *senderEntry = getRoutingEntryByHost(lsa->src);
        setLSADest(lsa, senderEntry->host, senderEntry->routingPort);
        addLSAtoBuffer(lsa);
        //Remove routing entry. Flood
        if(isZero) {
            removeRoutingEntry(nodeID);
            LSA *floodLSA = LSAfromLSA(lsa);
            unsigned int lastNodeID = getLastNodeID(lsa);
            addLSAWithDest(getLocalList(), floodLSA, lastNodeID);
        } else if(isMine && isHigher) {
            //I rebooted, catch up to higher seq. No flood.
            routingEntry *myEntry = getMyRoutingEntry();
            if(isNew) {
                myEntry->lastLSA = lsa;
            } else {
                myEntry->lastLSA->seqNo = lsa->seqNo;
            }
        } else if(isLower) {
            //neiggbor rebooted. Echo back his last LSA. No Flood
            routingEntry *entry = getRoutingEntry(nodeID);
            LSA *backLSA = LSAfromLSA(entry->lastLSA);
            addLSAWithOneDest(getLocalList(), backLSA, lastLSA->nodeID);
        } else if(isNew) {
            //Make new routing entry. Flood
            routingEntry *newEntry = malloc(sizeof(routingEntry));
            newEntry->nodeID = nodeID;
            newEntry->lastLSA = lsa;
            LSA *floodLSA= LSAfromLSA(lsa);
            decLSATTL(floodLSA);
            if(getLSATTL(floodLSA) > 0) {
                addLSAWithDest(getLocalList(), floodLSA, getLastNodeID(floodLSA));
            }
        } else if(isHigher) {
            //Update routing table. Flood 
            routingEntry *entry = getRoutingEntry(nodeID);
            replaceLSA(&(entry->lastLSA), lsa);
            //Flood if TTL > 0
            LSA *floodLSA = LSAfromLSA(lsa);
            decLSATTL(floodLSA);
            if(getLSATTL(floodLSA) > 0) {
                addLSAWithDest(getLocalList(), floodLSA, getLastNodeID(floodLSA));
            }
        }else{
            printf("What kind of LSA is this?!\n");
        }
    }
}

void getLSAFromRoutingTable(DLL *list)
{
    /* Add buffered LSA into list */
    localList = tRouting->LSAList;
    insertList(list, localList);
    while(localList->size > 0) {
        removeNode(localList, 0);
    }
    /* Add Advertisement LSA */
    newAdvertisement(list);
    /* Timeout old LSA */

    /* Retran LSA without ACK */
    
    /* Anything more to add? */
}

/* Routing Table */
unsigned int getLastNodeID(LSA *lsa)
{
    routingEntry *lastEntry = getRoutingEntryByHost(lsa->src);
    return lastEntry->nodeID;
}
void removeRoutingEntry(unsigned int nodeID)
{
    routingTable *tRou = tRouting;
    int i = 0;
    for(i = 0; i < tRou->table->size; i++) {
        routingEntry *entry = getNodeDataAt(tRou->table, i);
        if(entry->nodeID == nodeID) {
            removeNodeAt(tRou->table, i);
        }
    }
}

}
void updateTime()
{
    gettimeofday(&(tRouting->oldTime), NULL);
}

int initRoutingTable(int nodeID, char *rouFile, char *resFile,
                     int cycleTime, int neighborTimeout,
                     int retranTimeout, int LSATimeout)
{
    tRouting = malloc(sizeof(routingTable));

    tRouting->oldTime = malloc(sizeof(timeval));
    updateTime();
    tRouting->cycleTime = cycleTime;
    tRouting->neighborTimeout = neighborTimeout;
    tRouting->retranTimeout = retranTimeout;
    tRouting->LSATimeout = LSATimeout;

    tRouting->LSAList = malloc(sizeof(DLL));
    initList(tRouting->LSAList, compareLSA, freeLSA, NULL);
    tRouting->table = malloc(sizeof(DLL));
    initList(tRouting->table, compareRoutingEntry, freeRoutingEntry, NULL);
    return loadRoutingTable(tRouting, nodeID, rouFile, resFile);
}

int loadRoutingTable(routingTable *tRou, unsigned int nodeID, char *rouFile, char *resFile)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    fp = fopen(rouFile, "r");
    if(fp == NULL) {
        printf("Error reading routing table config.\n");
        return -1;
    }
    while(getline(&line, &len, fp) != -1) {
        printf("configLine: %s", line);
        routingEntry *re = parseRoutingLine(line);
        if(re->nodeID == nodeID) {
            re->isMe = 1;
            re->distance = 0;
            initResourceTable(re->tRes, resFile);
        } else {
            initResourceTable(re->tRes, NULL);
        }
        insertNode(tRou->table, re);
    }

    if(line != NULL) {
        free(line);
    }
    fclose(fp);
    return 0;
}

routingEntry *parseRoutingLine(char *line)
{
    int numMatch;
    routingEntry *newObj;
    unsigned int nodeID;
    char *host = NULL;
    int routingPort;
    int localPort;
    int serverPort;

    numMatch = sscanf(line, "%u %ms %d %d %d",
                      &nodeID, &host, &routingPort, &localPort, &serverPort);
    if(numMatch != 5) {
        if(host != NULL) {
            free(host);
        }
        return NULL;
    }
    newObj = malloc(sizeof(routingEntry));
    newObj->nodeID = nodeID;
    newObj->isMe = 0;
    newObj->host = host;
    newObj->routingPort = routingPort;
    newObj->localPort = localPort;
    newObj->serverPort = serverPort;
    newObj->tRes = malloc(sizeof(resourceTable));
    newObj->distance = 1;
    newObj->lastLSA = NULL;
    return newObj;
}


int getRoutingInfo(char *objName, routingInfo *rInfo)
{
    routingTable *tRou = tRouting;
    int i = 0;
    int minDistance = INT_MAX;
    char *path;
    int found = 0;
    for(i = 0; i < tRou->table->size; i++) {
        routingEntry *entry = getNodeDataAt(tRou->table, i);
        path = getPathByName(entry->tRes, objName);
        if(path != NULL) {
            found = 1;
            if(minDistance > entry->distance) {
                rInfo->host = entry->host;
                rInfo->port = entry->serverPort;
                rInfo->path = path;
                minDistance = entry->distance;
            }
        }
    }
    return found;
}

routingEntry *getRoutingEntryByHost(char *host)
{
    routingTable *tRou = tRouting;
    int i = 0;
    for(i = 0; i < tRou->table->size; i++) {
        routingEntry *entry = getNodeDataAt(tRou->table, i);
        if(entry->host != NULL && strcmp(entry->host, host) == 0) {
            return entry;
        }
    }
    return NULL;
}

routingEntry *getRoutingEntry(unsigned int nodeID)
{
    routingTable *tRou = tRouting;
    int i = 0;
    for(i = 0; i < tRou->table->size; i++) {
        routingEntry *entry = getNodeDataAt(tRou->table, i);
        if(entry->nodeID == nodeID) {
            return entry;
        }
    }
    return NULL;
}

routingEntry *getMyRoutingEntry()
{
    routingTable *tRou = tRouting;
    int i = 0;
    for(i = 0; i < tRou->table->size; i++) {
        routingEntry *entry = getNodeDataAt(tRou->table, i);
        if(entry->isMe) {
            return entry;
        }
    }
    return NULL;
}

int getRoutingPort(unsigned int nodeID)
{
    routingEntry *entry = getRoutingEntry(nodeID);
    if(entry == NULL) {
        return -1;
    } else {
        return entry->routingPort;
    }
}

int getLocalPort(unsigned int nodeID)
{
    routingEntry *entry = getRoutingEntry(nodeID);
    if(entry == NULL) {
        return -1;
    } else {
        return entry->localPort;
    }
}

void insertLocalResource(char *objName, char *objPath)
{
    routingEntry *entry = getMyRoutingEntry();
    insertResource(entry->tRes, objName, objPath);
}
