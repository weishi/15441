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

void newAdvertisement(int triggered)
{
    LSA *newAds = NULL;
    struct timeval curTime;
    struct timeval *oldTime = tRouting->oldTime;
    gettimeofday(&curTime, NULL);
    routingEntry *myEntry = getMyRoutingEntry();
    long diffTime = curTime.tv_sec - oldTime->tv_sec;
    printf("DiffTime = %ld, cycleTime = %d\n", diffTime, tRouting->cycleTime);
    int needAdv = diffTime > tRouting->cycleTime;
    if(myEntry->lastLSA == NULL) {
        newAds = newLSA(myEntry->nodeID, 0);
        fillLSAWithLink(newAds);
        fillLSAWithObj(newAds, myEntry->tRes);
        myEntry->lastLSA = newAds;
        printLSA(newAds);
        addLSAWithDest(getLocalLSABuffer(), newAds, 0);
        updateTime();
    } else if(needAdv || triggered) {
        incLSASeq(myEntry->lastLSA);
        newAds = headerLSAfromLSA(myEntry->lastLSA);
        fillLSAWithLink(newAds);
        fillLSAWithObj(newAds, myEntry->tRes);
        replaceLSA(&(myEntry->lastLSA), newAds);
        addLSAWithDest(getLocalLSABuffer(), newAds, 0);
        updateTime();
    }
}

void addLSAWithDest(DLL *list, LSA *lsa, unsigned int ignoreID)
{
    DLL *table = tRouting->table;
    int i = 0;
    while(i < table->size) {
        routingEntry *entry = getNodeDataAt(table, i);
        if(entry->distance == 1 && entry->nodeID != ignoreID) {
            LSA *thisLSA = LSAfromLSA(lsa);
            setLSADest(thisLSA, entry->host, entry->routingPort);
            printf("LSA Dest: %s:%d\n", thisLSA->dest, thisLSA->destPort);
            insertNode(list, thisLSA);
            insertNode(entry->ackPool, thisLSA);
        }
        i++;
    }
}
void addLSAWithOneDest(DLL *list, LSA *lsa, unsigned int destID)
{
    routingEntry *entry = getRoutingEntry(destID);
    LSA *thisLSA = LSAfromLSA(lsa);
    setLSADest(thisLSA, entry->host, entry->routingPort);
    insertNode(list, thisLSA);
    insertNode(entry->ackPool, thisLSA);
}

void fillLSAWithLink(LSA *lsa)
{
    DLL *table = tRouting->table;
    int i = 0;
    while(i < table->size) {
        routingEntry *entry = getNodeDataAt(table, i);
        if(entry->distance == 1) {
            insertLSALink(lsa, entry->nodeID);
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
        routingEntry *senderEntry = getRoutingEntryByHost(lsa->src, lsa->srcPort);
        DLL *ackPool = senderEntry->ackPool;
        int i = 0;
        int found = 0;
        while(i < ackPool->size) {
            LSA *thisLSA = getNodeDataAt(ackPool, i);
            found = (thisLSA->seqNo == lsa->seqNo) &&
                    (thisLSA->senderID == lsa->senderID);
            if(found) {
                removeNodeAt(ackPool, i);
                printf("LSA ACK checked in.\n");
                found = 1;
                break;
            }
            i++;
        }
        if(!found) {
            printf("Got ACK for something we didn't send...\n");
        }
        freeLSA(lsa);
    } else {
        routingEntry *entry = getRoutingEntry(nodeID);
        routingEntry *myEntry = getMyRoutingEntry();
        isZero = (lsa->TTL == 0);
        isNew = (entry == NULL || entry->lastLSA == NULL) && !isZero;
        isHigher =  !isNew && !isZero && entry != NULL &&
                    entry->lastLSA->seqNo < lsa->seqNo;
        isSame =    !isNew && !isZero && entry != NULL &&
                    entry->lastLSA->seqNo == lsa->seqNo;
        isLower =   !isNew && !isZero && entry != NULL &&
                    entry->lastLSA->seqNo > lsa->seqNo;
        isMine = (entry != NULL &&  entry == myEntry);
        //Send back ACK (ANY)
        LSA *ack = headerLSAfromLSA(lsa);
        setLSAAck(ack);
        setLSADest(ack, lsa->src, lsa->srcPort);
        addLSAtoBuffer(ack);
        if(isZero) {
            if(entry->distance != 0) {
                //Remove routing entry. Flood
                printf("isZero\n");
                if(entry->distance == -1 || entry->distance == 1) {
                    entry->distance = -1;
                } else {
                    removeRoutingEntry(nodeID);
                }
                addLSAWithDest(getLocalLSABuffer(), lsa, getLastNodeID(lsa));
            } else {
                printf("I'm not dead anymore. Ignore old news.\n");
            }
            freeLSA(lsa);
        } else if(isMine && isHigher) {
            printf("isMine & higher\n");
            //I rebooted, catch up to higher seq. No flood.
            if(isNew) {
                myEntry->lastLSA = lsa;
            } else {
                myEntry->lastLSA->seqNo = lsa->seqNo;
                freeLSA(lsa);
            }
        } else if(isLower) {
            printf("isLower\n");
            //neiggbor rebooted. Echo back his last LSA. No Flood
            LSA *backLSA = entry->lastLSA;
            backLSA->isDown = 0;
            backLSA->isExpired = 0;
            entry->distance = 1;
            addLSAWithOneDest(getLocalLSABuffer(), backLSA, backLSA->senderID);
            freeLSA(lsa);
        } else if(isNew) {
            //Make new routing entry. Flood
            printf("isNew\n");
            routingEntry *newEntry;
            if(entry == NULL) {
                newEntry = malloc(sizeof(routingEntry));
                newEntry->nodeID = nodeID;
                newEntry->isMe = 1;
                newEntry->host = NULL;
                newEntry->routingPort = 0;
                newEntry->localPort = 0;
                newEntry->serverPort = 0;
                newEntry->tRes = NULL;
                newEntry->distance = 2;
                insertNode(tRouting->table, newEntry);
            } else {
                newEntry = entry;
                if(entry->distance == -1) {
                    entry->distance = 1;
                }
            }
            newEntry->lastLSA = lsa;
            LSA *floodLSA = LSAfromLSA(lsa);
            decLSATTL(floodLSA);
            if(getLSATTL(floodLSA) > 0) {
                addLSAWithDest(getLocalLSABuffer(), floodLSA, getLastNodeID(lsa));
            }
            freeLSA(floodLSA);
        } else if(isHigher) {
            printf("isHigher\n");
            //Update routing table. Flood
            replaceLSA(&(entry->lastLSA), lsa);
            //Flood if TTL > 0
            LSA *floodLSA = LSAfromLSA(lsa);
            decLSATTL(floodLSA);
            if(getLSATTL(floodLSA) > 0) {
                addLSAWithDest(getLocalLSABuffer(), floodLSA, getLastNodeID(floodLSA));
            }
            freeLSA(floodLSA);
        } else {
            printf("@@@Ignored@@@\n");
            freeLSA(lsa);
        }
    }
}

void getLSAFromRoutingTable(DLL **list)
{
    /* Add Advertisement LSA */
    newAdvertisement(0);
    /* Retran LSA without ACK */
    checkNeighborDown();
    /* Timeout old LSA */
    expireOldLSA();
    /* Add buffered LSA into list */
    DLL *localList = getLocalLSABuffer();
    *list = localList;
    /* Print status */
    printRoutingTable();
    /* TODO:Anything more to add? */

}



/* Routing Table */
void printRoutingTable()
{
    DLL *table = tRouting->table;
    int i;
    unsigned int j;
    printf("++++++ \tRoutingTable\t ++++++\n");
    for(i = 0; i < table->size; i++) {
        routingEntry *entry = getNodeDataAt(table, i);
        LSA *lsa = entry->lastLSA;
        printf("[%d]Dist=%d;ACK=%d;",
               entry->nodeID, entry->distance, entry->ackPool->size);
        if(lsa != NULL) {
            printf("Seq=%d;Exp=%d;Link:", lsa->seqNo, lsa->isExpired);
            for(j = 0; j < lsa->numLink; j++) {
                printf("%d,", lsa->listLink[j]);
            }
            printf("-Obj:");
            for(j = 0; j < lsa->numObj; j++) {
                printf("%s,", (char *)getNodeDataAt(lsa->listObj, j));
            }
        }
        printf("\n");
    }
}

unsigned int getLastNodeID(LSA *lsa)
{
    routingEntry *lastEntry = getRoutingEntryByHost(lsa->src, lsa->srcPort);
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

void updateTime()
{
    gettimeofday(tRouting->oldTime, NULL);
}

int initRoutingTable(int nodeID, char *rouFile, char *resFile,
                     int cycleTime, int neighborTimeout,
                     int retranTimeout, int LSATimeout)
{
    tRouting = malloc(sizeof(routingTable));

    tRouting->oldTime = malloc(sizeof(struct timeval));
    updateTime();
    tRouting->cycleTime = cycleTime;
    tRouting->neighborTimeout = neighborTimeout;
    tRouting->retranTimeout = retranTimeout;
    tRouting->LSATimeout = LSATimeout;

    tRouting->LSAList = malloc(sizeof(DLL));
    initList(tRouting->LSAList, compareLSA, NULL, NULL, copyLSA);
    tRouting->table = malloc(sizeof(DLL));
    initList(tRouting->table,
             compareRoutingEntry, freeRoutingEntry, NULL, NULL);
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
    newObj->ackPool = malloc(sizeof(DLL));
    initList(newObj->ackPool, compareLSA, freeLSA, NULL, NULL);
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

routingEntry *getRoutingEntryByHost(char *host, int port)
{
    routingTable *tRou = tRouting;
    int i = 0;
    printf("Searching:%s:%d", host, port);
    for(i = 0; i < tRou->table->size; i++) {
        routingEntry *entry = getNodeDataAt(tRou->table, i);
        if(entry->host != NULL &&
                entry->routingPort == port &&
                strcmp(entry->host, host) == 0) {
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

DLL *getLocalLSABuffer()
{
    return tRouting->LSAList;
}

void insertLocalResource(char *objName, char *objPath)
{
    routingEntry *entry = getMyRoutingEntry();
    insertResource(entry->tRes, objName, objPath);
    newAdvertisement(1);
}

void addLSAtoBuffer(LSA *lsa)
{
    DLL *list = getLocalLSABuffer();
    insertNode(list, lsa);
}

void expireOldLSA()
{
    DLL *table = tRouting->table;
    routingEntry *entry;
    int i;
    long diffTime;
    LSA *lsa;
    struct timeval curTime;
    gettimeofday(&curTime, NULL);
    for(i = 0; i < table->size; i++) {
        entry = getNodeDataAt(table, i);
        lsa = entry->lastLSA;
        if(lsa != NULL) {
            diffTime = curTime.tv_sec - (lsa->timestamp).tv_sec;
            if(diffTime >= tRouting->LSATimeout) {
                printf("Expire node[%d]\n", entry->nodeID);
                entry->lastLSA->isExpired = 1;
            }
        }
    }
}

void checkNeighborDown()
{
    DLL *table = tRouting->table;
    routingEntry *entry;
    int i, j;
    int needRetran, isDown;
    long diffTime;
    LSA *lsa;
    struct timeval curTime;
    gettimeofday(&curTime, NULL);
    for(i = 0; i < table->size; i++) {
        entry = getNodeDataAt(table, i);
        if(entry->distance == 1) {
            DLL *ackPool = entry->ackPool;
            for(j = 0; j < ackPool->size; j++) {
                lsa = getNodeDataAt(ackPool, j);
                diffTime = curTime.tv_sec - lsa->timestamp.tv_sec;
                needRetran = diffTime > tRouting->retranTimeout
                             && !hasLSARetran(lsa)
                             && diffTime < tRouting->neighborTimeout;
                isDown = diffTime >= tRouting->neighborTimeout
                         && !isLSADown(lsa);
                if(needRetran) {
                    LSA *retranLSA = LSAfromLSA(lsa);
                    addLSAWithOneDest(getLocalLSABuffer(), retranLSA, entry->nodeID);
                    setLSARetran(lsa);
                    printf("Neighbor[%d] needs retran\n", entry->nodeID);
                    freeLSA(retranLSA);
                } else if(isDown) {
                    entry->distance = -1;
                    //Clean up all queued ACK for this node
                    while(ackPool->size > 0) {
                        //Remove it from Outcoming Queue, if exists
                        DLL *localList = getLocalLSABuffer();
                        int k = 0;
                        for(k = 0; k < localList->size; k++) {
                            LSA *lsa1 = getNodeDataAt(localList, k);
                            LSA *lsa2 = getNodeDataAt(ackPool, 0);
                            if(compareLSA(lsa1, lsa2) == 0) {
                                removeNodeAt(localList, k);
                                break;
                            }
                        }
                        removeNodeAt(ackPool, 0);
                    }
                    if(entry->lastLSA != NULL) {
                        entry->lastLSA->isDown = 1;
                        LSA *downLSA = LSAfromLSA(entry->lastLSA);
                        setLSADown(downLSA); //set TTL=0
                        addLSAWithDest(getLocalLSABuffer(), downLSA, entry->nodeID);
                        freeLSA(downLSA);
                    }
                    printf("Neighbor[%d] is down\n", entry->nodeID);
                } else {
                }
            }
        }
    }
}










