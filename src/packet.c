#include "packet.h"

void freePacket(Packet *pkt)
{
  if(pkt->dest != NULL)
    free(pkt->dest);
  free(pkt);
}

int verifyPacket(Packet *pkt)
{
    uint16_t magic = getPacketMagic(pkt);
    uint8_t version = getPacketVersion(pkt);
    
    if(magic == 15441 && version == 1) {
        return 1;
    } else {
        printf("Invalid magic=%d, version=%d\n", magic, version);
        return 0;
    }
}

Packet *newPacketDefault()
{
    Packet *pkt = calloc(sizeof(Packet), 1);
    memset(pkt, 0, sizeof(Packet));
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

uint16_t getPacketMagic(Packet *pkt)
{
    uint8_t *ptr = (pkt->payload);
    return *((uint16_t *)ptr);
}

uint8_t getPacketVersion(Packet *pkt)
{
    uint8_t *ptr = (pkt->payload);
    return *((uint8_t *)(ptr + 2));
}

uint8_t getPacketType(Packet *pkt)
{
    uint8_t *ptr = (pkt->payload);
    return *((uint8_t *)(ptr + 3));
}

uint16_t getPacketSize(Packet *pkt)
{
    uint8_t *ptr = (pkt->payload);
    return *((uint16_t *)(ptr + 6));
}

uint32_t getPacketSeq(Packet *pkt)
{
    uint8_t *ptr = (pkt->payload);
    return *((uint32_t *)(ptr + 8));
}

uint32_t getPacketAck(Packet *pkt)
{
    uint8_t *ptr = (pkt->payload);
    return *((uint32_t *)(ptr + 12));
}

void setPacketSeq(Packet *pkt, uint32_t seqNo)
{
    uint8_t *ptr = (pkt->payload);
    *((uint32_t *)(ptr + 8)) = seqNo;
}

void setPacketAck(Packet *pkt, uint32_t ackNo)
{
    uint8_t *ptr = (pkt->payload);
    *((uint32_t *)(ptr + 12)) = ackNo;
}

void setPacketSize(Packet *pkt, uint16_t size)
{
    uint8_t *ptr = (pkt->payload);
    *((uint16_t *)(ptr + 6)) = size;
}

void incPacketSize(Packet *pkt, uint16_t size)
{
    uint8_t *ptr = (pkt->payload);
    *((uint16_t *)(ptr + 6)) += size;
}

void setPacketTime(Packet *pkt)
{
    gettimeofday(&(pkt->timestamp), NULL);
}

void newPacketWHOHAS(queue *sendQueue)
{
    int i, j;
    int pktIndex = 0;
    int needToFetch = 0;
    int numHash = getChunk.numChunk;
    int numPacket = numHash / MAX_HASH_PER_PACKET;
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
	while(getChunk.list[pktIndex].fetchState == 1
	      || (searchHash(getChunk.list[pktIndex].hash, &hasChunk, -1) >= 0)) {
	  pktIndex++;
	}
	if(pktIndex == numHash)
	  break;
	insertPacketHash(thisObj, getChunk.list[pktIndex].hash);
	needToFetch = 1;
	pktIndex++;
      }
      if(needToFetch)
	enqueue(sendQueue, (void *)thisObj);
    }
}

int newPacketGET(Packet *pkt, queue *getQueue)
{
  int ret = 0;
  uint8_t numHash = getPacketNumHash(pkt);
  int i, idx;
  uint8_t *hash;
  for(i = 0; i < numHash; i++) {
    hash = getPacketHash(pkt, i);
    printHash(hash);
    idx = searchHash(hash, &getChunk, -1);
    printf("idx %d hashSeq %d getState %d\n", idx, getChunk.list[idx].seq, getChunk.list[idx].fetchState);
    //Only GET when chunk hasn't been fetched
    if(idx >= 0 && getChunk.list[idx].fetchState == 0) {
      printf("geting chunk %d\n",getChunk.list[idx].seq);
      Packet *thisObj = newPacketSingleGET(hash);
      enqueue(getQueue, (void *)thisObj);
      ret = 1;
    }
  }
  return ret;
}

void newPacketACK(uint32_t ack, queue *ackSendQueue)
{
    Packet *thisObj = newPacketDefault();
    setPacketType(thisObj, "ACK");
    setPacketAck(thisObj, ack);
    enqueue(ackSendQueue, (void *)thisObj);
}

Packet *newFreePacketACK(uint32_t ack){
  Packet *thisObj = newPacketDefault();
  setPacketType(thisObj, "ACK");
  setPacketAck(thisObj, ack);
  return thisObj;
}

void newPacketDATA(Packet *pkt, queue *dataQueue)
{
  uint8_t *hash = pkt->payload + 16;
  int idx = searchHash(hash, &masterChunk, -1);
  Packet *newPkt;
  if(idx >= 0) {
    int i = 0;
    int numPacket = BT_CHUNK_SIZE / PACKET_DATA_SIZE;
    if(BT_CHUNK_SIZE % PACKET_DATA_SIZE > 0) {
      numPacket++;
    }
    for(i = 0; i < numPacket; i++) {
      if(i == numPacket - 1) {
	newPkt = newPacketSingleDATA(i + 1, idx, BT_CHUNK_SIZE % PACKET_DATA_SIZE);
      } else {
	newPkt = newPacketSingleDATA(i + 1, idx, PACKET_DATA_SIZE);
      }
      enqueue(dataQueue, newPkt);
    }
  }
}

Packet *newPacketSingleDATA(int seqNo, int seqChunk, size_t size)
{
    size_t retSize;
    long offset = seqChunk * BT_CHUNK_SIZE + (seqNo - 1) * PACKET_DATA_SIZE;
    Packet *pkt = newPacketDefault();

    setPacketType(pkt, "DATA");
    incPacketSize(pkt, size);
    setPacketSeq(pkt, seqNo);

    FILE *mf = masterChunk.filePtr;

    rewind(mf);
    fseek(mf, offset, SEEK_SET);
    retSize = fread(pkt->payload + 16, sizeof(uint8_t), size, mf);
    if(retSize != size) {
      printf("IO Error reading chunk\n");
      freePacket(pkt);
      return NULL;
    } else {
      return pkt;
    }
}

Packet *newPacketSingleGET(uint8_t *hash)
{
    Packet *thisObj = newPacketDefault();
    incPacketSize(thisObj, 20);
    setPacketType(thisObj, "GET");
    memcpy(thisObj->payload + 16, hash, SHA1_HASH_SIZE);
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
        idx = searchHash(hash, &hasChunk, -1);
        if(idx >= 0) {
            printf("Has[%d]", i);
            insertPacketHash(thisObj, hash);
        }
    }

    if(i == 0 || getPacketSize(thisObj) == 20) {
        freePacket(thisObj);
        return NULL;
    } else {
      setPacketDest(thisObj, &(pktWHOHAS->src), sizeof(pktWHOHAS->src));
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
    if(type == 0 || type == 1){ //WHOHAS and IHAVE
      return pkt->payload + 20 + SHA1_HASH_SIZE * i;
    } else if (type == 2){ //GET
      return pkt->payload + 16 + SHA1_HASH_SIZE * i;
    } else {
        return NULL;
    }
}
void insertPacketHash(Packet *pkt, uint8_t *hash)
{
    uint8_t *ptr = pkt->payload;
    uint8_t numHash = *(ptr + 16);
    memcpy(ptr + 20 + numHash * SHA1_HASH_SIZE, hash, SHA1_HASH_SIZE);
    *(ptr + 16) = numHash + 1;//increment numHash
    incPacketSize(pkt, SHA1_HASH_SIZE);
}



Packet *newPacketFromBuffer(char *buf)
{

    Packet *newObj = malloc(sizeof(Packet));
    memset(newObj, 0, sizeof(Packet));
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



int searchHash(uint8_t *hash, chunkList *chunkPool, int fetchState)
{
    int i;
    for(i = 0; i < chunkPool->numChunk; i++) {
        chunkLine *line = &(chunkPool->list[i]);
        int matched = sameHash(line->hash, hash, SHA1_HASH_SIZE);
        if(fetchState >= 0) {
            matched = matched && line->fetchState == fetchState;
        }
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

void setPacketDest(Packet* pkt, struct sockaddr_in *addr, int length){
  pkt->dest = malloc(length);
  memcpy(pkt->dest, addr, length);
}
void printHash(uint8_t *hash)
{
    char buf[50];
    bzero(buf, 50);
    binary2hex(hash, 20, buf);
    printf("%s\n", buf);
}
