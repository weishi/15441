#include "packet.h"

void freePacket(Packet *pkt)
{
    free(pkt);
}

int verifyPacket(Packet *pkt)
{
    uint16_t magic = getPacketMagic(pkt);
    uint8_t version = getPacketVersion(pkt);

    if(magic == 15441 && version == 1) {
        return 1;
    } else {
        return 0;
    }
}

Packet *newPacketDefault()
{
    Packet *pkt = calloc(sizeof(Packet), 1);
    uint8_t *ptr = pkt->payload;
    *((uint16_t *)ptr) = 15441; //magic number
    *(ptr + 2) = 1; //version
    *((uint16_t *)(ptr + 4)) = 16; //Header size
    *((uint16_t *)(ptr + 6)) = 16; //Packet size
    return pkt;
}

void setPacketType(Packet *pkt, char *type)
{
    uint8_t *ptr = pkt->payload;
    uint8_t numType = 6;
    if(strcmp(type, "WHOHAS") == 0) {
        numType = 0;
    } else if(strcmp(type, "IHAVE") == 0) {
        numType = 1;
    } else if(strcmp(type, "GET") == 0) {
        numType = 2;
    } else if(strcmp(type, "DATA") == 0) {
        numType = 3;
    } else if(strcmp(type, "ACK") == 0) {
        numType = 4;
    } else if(strcmp(type, "DENIED") == 0) {
        numType = 5;
    }
    *(ptr + 3) = numType;
}
uint16_t getPacketSize(Packet *pkt)
{
    uint8_t *ptr = (pkt->payload);
    return *((uint16_t *)(ptr + 6));
}

uint16_t getPacketMagic(Packet *pkt)
{
    uint8_t *ptr = (pkt->payload);
    return *((uint16_t *)ptr);
}

uint8_t getPacketVersion(Packet *pkt)
{
    uint8_t *ptr = (pkt->payload);
    return *((uint8_t *)(ptr + 3));
}

uint8_t getPacketType(Packet *pkt)
{
    uint8_t *ptr = (pkt->payload);
    return *((uint8_t *)(ptr + 4));
}

void setPacketSize(Packet *pkt, uint16_t size)
{
    uint8_t *ptr = (pkt->payload);
    *((uint16_t *)(ptr + 6)) = size; //Packet size
}

void incPacketSize(Packet *pkt, uint16_t size)
{
    uint8_t *ptr = (pkt->payload);
    *((uint16_t *)(ptr + 6)) += size; //Packet size
}

void newPacketWHOHAS(queue *sendQueue)
{
    int i, j;
    int pktIndex = 0;
    int numPacket = getChunk.numChunk / MAX_HASH_PER_PACKET;
    if(getChunk.numChunk % MAX_HASH_PER_PACKET > 0) {
        numPacket++;
    }

    for(i = 0; i < numPacket; i++) {
        int num;
        Packet *thisObj = newPacketDefault();
        incPacketSize(thisObj, 4);
        setPacketType(thisObj, "WHOHAS");
        if(i < numPacket - 1) {
            num = MAX_HASH_PER_PACKET;
        } else {
            num = getChunk.numChunk % MAX_HASH_PER_PACKET;
        }
        for(j = 0; j < num; j++) {
            while(getChunk.list[pktIndex].fetchState) {
                pktIndex++;
            }
            insertPacketHash(thisObj, getChunk.list[pktIndex].hash);
        }
        enqueue(sendQueue, (void *)thisObj);
    }
}

void newPacketGET(Packet *pkt, queue *getQueue)
{
    uint8_t numHash = getPacketNumHash(pkt);
    int i, idx;
    uint8_t *hash;
    for(i = 0; i < numHash; i++) {
        hash = getPacketHash(pkt, i);
        idx = searchHash(hash, &getChunk);
        //Only GET when chunk hasn't been fetched
        if(getChunk.list[idx].fetchState == 0) {
            getChunk.list[idx].fetchState = 2;
            Packet *thisObj = newPacketSingleGET(hash);
            enqueue(getQueue, (void*)thisObj);
        }
    }
}

Packet *newPacketSingleGET(uint8_t *hash)
{
    Packet *thisObj = newPacketDefault();
    incPacketSize(thisObj, 20);
    memcpy(&(thisObj->payload) + 16, hash, SHA1_HASH_SIZE);
    return thisObj;
}


Packet *newPacketIHAVE(Packet *pktWHOHAS)
{
    uint8_t numHash = getPacketNumHash(pktWHOHAS);
    int i = 0;
    int idx;
    uint8_t *hash;
    Packet *thisObj = newPacketDefault();
    incPacketSize(thisObj, 4);
    setPacketType(thisObj, "IHAVE");
    for(i = 0; i < numHash; i++) {
        hash = getPacketHash(pktWHOHAS, i);
        idx = searchHash(hash, &hasChunk);
        insertPacketHash(thisObj, hash);
    }

    if(i == 0) {
        freePacket(thisObj);
        return NULL;
    } else {
        return thisObj;
    }
}

uint8_t getPacketNumHash(Packet *pkt)
{
    uint8_t *ptr = (pkt->payload);
    return *((uint8_t *)(ptr + 16));
}

uint8_t *getPacketHash(Packet *pkt, int i)
{
    int type = getPacketType(pkt);
    if(type == 0 || type == 1 || type == 2) {
        uint8_t *pPtr = (pkt->payload);
        return pPtr + 20 + SHA1_HASH_SIZE * i;
    } else {
        return NULL;
    }
}
void insertPacketHash(Packet *pkt, uint8_t *hash)
{
    uint8_t *ptr = (uint8_t *)pkt;
    uint8_t numHash = *(ptr + 16);
    memcpy(ptr + 20 + numHash * SHA1_HASH_SIZE, hash, SHA1_HASH_SIZE);
    *(ptr + 16) = numHash + 1;//increment numHash
    incPacketSize(pkt, SHA1_HASH_SIZE);
}



Packet *newPacketFromBuffer(char *buf)
{

    Packet *newObj = malloc(sizeof(Packet));
    memcpy(newObj->payload, buf, 1500);
    //printPacket(newObj);
    return newObj;
}
/*
void printPacket(Packet *pkt)
{
    printf("==Packet== ");
    printf("src=%s:%d;dest=%s:%d;", pkt->src, pkt->srcPort, pkt->dest, pkt->destPort);
    printf("version=%d;TTL=%d;type=%d;", pkt->version, pkt->TTL, pkt->type);
    printf("senderID=%d;seqNo=%d;", pkt->senderID, pkt->seqNo);
    printf("numLink=%d;numObj=%d;", pkt->numLink, pkt->numObj);
    printf("hasRetran=%d;isDown=%d", pkt->hasRetran, pkt->isDown);

}
*/



int searchHash(uint8_t *hash, chunkList *chunkPool)
{
    int i;
    for(i = 0; i < chunkPool->numChunk; i++) {
        chunkLine *line = &(chunkPool->list[i]);
        int matched = line->fetchState == 0 &&
                      sameHash(line->hash, hash, SHA1_HASH_SIZE);
        if(matched) {
            return i;
        }
    }
    return -1;
}

int sameHash(uint8_t *hash1, uint8_t *hash2, int size)
{
    int i;
    for(i = 0; i < size; i++) {
        if(hash1[i] != hash2[i]) {
            return 0;
        }
    }
    return 1;
}


