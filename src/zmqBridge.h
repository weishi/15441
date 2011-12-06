#ifndef ZMQBRIDGE_H
#define ZMQBRIDGE_H

#define MAX_NUM_CHUNK 1024

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "zmq.h"
#include "queue.h"

#define INT_PORT1 55554
#define INT_PORT2 55553
#define DATASIZE 1484

typedef struct dataChunk{
    char *data;
    int curSize;
    int size;
}dataChunk;

typedef struct dataPacket{
    uint32_t id;
    uint32_t size;
    uint32_t allSize;
    uint32_t ack;
    char data[DATASIZE];
}dataPacket;

queue* subq;
queue* pubq;

queue *outDC;
dataPacket *waitACK;
dataPacket *outACK;
dataChunk *inDC;

struct sockaddr_in client;
struct sockaddr_in server;



void startBridge(void *, void *);
int openSocket(int port);

void handleSUB(void *);
void handlePUB(void *);
void handleServerData(int);
void handleServerACK(int);
void handleClientData(int);
void handleClientACK(int);

void chunkifyDC(queue *q, dataChunk *dc);

void freeDataChunk(dataChunk *dc);
#endif
