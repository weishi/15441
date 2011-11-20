#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <netinet/in.h>


#include "linkedList.h"

typedef struct Packet {
    char *src;
    char *dest;
    int srcPort;
    int destPort;
    struct timeval timestamp;
    //Header
    uint16_t magicNum;
    uint8_t version;
    uint8_t type;
    uint16_t headerLength;
    uint16_t packetLengh;
    uint32_t seqNo;
    uint32_t ackNo;
    //Payload(IHAVE/WHOHAS)
    uint32_t numObj;
    DLL *listObj;
    //Payload(DATA)
    char *data;
} Packet;

/* Constructor */
Packet *PacketfromBuffer(char *, ssize_t, char *, int);
Packet *PacketfromPacket(Packet *);
Packet *headerPacketfromPacket(Packet *);
Packet *newPacket(uint32_t, uint32_t);

int comparePacket(void *data1, void *data2);
void freePacket(void *data);
void *copyPacket(void *data);

void PackettoBuffer(Packet *, char *, ssize_t*);

void replacePacket(Packet **, Packet *);
/* Getters and Setters */
void incPacketSeq(Packet *);
void setPacketDest(Packet *, char*, int);

void insertPacketLink(Packet *, uint32_t);
void insertPacketObj(Packet *, char *);


#endif
