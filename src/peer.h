#ifndef _PEER_H_
#define _PEER_H_

#define MAX_NUM_CHUNK 1024
#define MAX_NUM_PEER 1024
#define MAX_LINE_SIZE 1024
#define GET_TIMEOUT_SEC 5
#define DATA_TIMEOUT_SEC 3

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "debug.h"
#include "spiffy.h"
#include "bt_parse.h"
#include "input_buffer.h"
#include "chunk.h"

#include "window.h"
#include "connPool.h"
#include "chunkList.h"
#include "packet.h"

typedef struct peerList_t {
  int peerID;
  int isMe;
  struct sockaddr_in addr;
} peerList_t;

typedef struct peerInfo_t {
  int numPeer;
  peerList_t peerList[MAX_NUM_PEER];
} peerInfo_t;

peerInfo_t peerInfo;

extern chunkList masterChunk;
extern chunkList hasChunk;
extern chunkList getChunk;
extern FILE *log_file;

int idle = 1;
/* Connection */
queue *nonCongestQueue;//For WHOHAS,IHAVE

int maxConn;
connUp uploadPool[MAX_NUM_PEER];
connDown downloadPool[MAX_NUM_PEER];

void init(bt_config_t *);
void printInit();
void printChunk(chunkList *);
void fillChunkList(chunkList *, enum chunkType, char *);
void fillPeerList(bt_config_t *);

void handlePacket(Packet *);
int searchPeer(struct sockaddr_in *);

void flushQueue(int , queue *);

void flushUpload(int sock );//DATA, ACK
void flushDownload(int sock );//GET

long diffTimeval(struct timeval *t1, struct timeval *t2);
int diffTimevalMilli(struct timeval *t1, struct timeval *t2);
int updateGetSingleChunk(Packet *, int );
void updateGetChunk();
void updateACKQueue(Packet *, int);

#endif
