#ifndef _PEER_H_
#define _PEER_H_

#define MAX_NUM_CHUNK 1024
#define MAX_NUM_PEER 1024
#define SHA1_HASH_SIZE 20
#define MAX_LINE_SIZE 1024

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "spiffy.h"
#include "bt_parse.h"
#include "input_buffer.h"
#include "chunk.h"

#include "window.h"
#include "connPool.h"

enum chunkType{
    MASTER,
    GET,
    HAS,
};

typedef struct chunkLine{
    int seq;
    uint8_t hash[SHA1_HASH_SIZE];
}chunkLine;

typedef struct chunkList{
	enum chunkType type;
	int numChunk;
	chunkLine list[MAX_NUM_CHUNK];
    FILE *filePtr;//Master input or GET output
}chunkList;

typedef struct peerList_t{
    int peerID;
    int isMe;
    struct sockaddr_in addr;
}peerList_t;

/* Chunk */
chunkList masterChunk;
chunkList hasChunk;
chunkList getChunk;

peerList_t peerList[MAX_NUM_PEER];

/* Connection */
int maxConn;
connUp uploadPool[MAX_NUM_PEER];
connDown downlaodPool[MAX_NUM_PEER];

void init(bt_config_t *);
void fillChunkList(chunkList *, enum chunkType, char *);
void fillPeerList(bt_config_t *);

#endif
