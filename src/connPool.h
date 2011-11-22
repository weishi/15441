#ifndef CONNPOOL_H
#define CONNPOOL_H 

#include "window.h"

typedef struct connUp{
    queue *dataQueue;
    queue *ackWaitQueue;
    sendWindow sw; 
}connUp;


typedef struct connDown{
    //0:ready for next
    //1:waiting
    //2:downloading
    int state; 
    int curChunkID;
    queue *getQueue;
    queue *timeoutQueue;
    queue *ackSendQueue;
    recvWindow rw;
}connDown;


#endif
