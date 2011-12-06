#ifndef PACKET_H
#define PACKET_H

#define MAX_HASH_PER_PACKET ((1500-20)/20)
#define PACKET_DATA_SIZE (1500-16)


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <netinet/in.h>

#include "queue.h"
#include "chunkList.h"
#include "chunk.h"

extern chunkList masterChunk;
extern chunkList getChunk;
extern chunkList hasChunk;

typedef struct Packet {
  struct sockaddr_in src;
  struct sockaddr_in* dest; //used only for IHAVE packet
  struct timeval timestamp;
  uint8_t payload[1500];
} Packet;

/* Constructor */
Packet *newPacketFromBuffer(char *);
Packet *newPacketDefault();
void newPacketWHOHAS(queue *);
int newPacketGET(Packet *,queue *); //return 1 if a packet actually gets pushed into the getqueue and 0 otherwise
void newPacketDATA(Packet *,queue *);
void newPacketACK(uint32_t,queue *);
Packet * newFreePacketACK(uint32_t);
Packet * newPacketSingleDATA(int , int , size_t);
Packet * newPacketSingleGET(uint8_t*);
Packet *newPacketIHAVE(Packet *);

void freePacket(Packet *data);

int verifyPacket(Packet *);
void PackettoBuffer(Packet *, char *, ssize_t*);

/* Getters and Setters */
uint32_t getPacketSeq(Packet *pkt);
uint32_t getPacketAck(Packet *pkt);
uint16_t getPacketSize(Packet *pkt);
uint16_t getPacketMagic(Packet *pkt);
uint8_t getPacketVersion(Packet *pkt);
uint8_t getPacketType(Packet *pkt);
uint8_t getPacketNumHash(Packet *pkt);
uint8_t *getPacketHash(Packet *pkt, int);

void setPacketType(Packet *pkt, char *);
void setPacketSize(Packet *pkt, uint16_t);
void incPacketSize(Packet *pkt, uint16_t);
void setPacketSeq(Packet *pkt, uint32_t);
void setPacketAck(Packet *pkt, uint32_t);
void setPacketTime(Packet *pkt);

void setPacketDest(Packet *, struct sockaddr_in*, int);

void insertPacketData(Packet *, char *);
void insertPacketHash(Packet *, uint8_t *);

/* Hash related methods */

int searchHash(uint8_t *hash, chunkList *chunkPool,int);
int sameHash(uint8_t *hash1, uint8_t *hash2, int size);

void printHash(uint8_t*); 

#endif
