#ifndef CONNPOOL_H
#define CONNPOOL_H 

#include "window.h"
#include "sortedPacketCache.h"

typedef struct connUp{
  sendWindow sw;
  struct timeval startTime;
  queue *dataQueue;
  queue *ackWaitQueue;
  uint32_t connID;
  uint8_t connected;
  uint8_t timeoutCount; // assert connection loss upon 3 consecutive timeouts
}connUp;


typedef struct connDown{
  //0:ready for next
    //1:downloading
    //2:waiting for connection
  recvWindow rw;
  int state; 
  int curChunkID;
  queue *getQueue;
  queue *timeoutQueue; //GET request timeout queue
  queue *ackSendQueue;
  sortedPacketCache* cache;
  uint8_t connected;
  uint8_t timeoutCount; // assert connection loss upon 3 consecutive timeouts
}connDown;

void cleanUpConnUp(connUp *conn);
void cleanUpConnDown(connDown *conn);

#endif
