#ifndef CONNPOOL_H
#define CONNPOOL_H 

#include "queue.h"

typedef struct connUp{
}connUp;


typedef struct connDown{
    queue getQueue;
    queue ackQueue;
}connDown;


#endif
