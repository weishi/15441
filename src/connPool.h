#ifndef CONNPOOL_H
#define CONNPOOL_H 


typedef struct connUp{
}connUp;


typedef struct connDown{
    queue getQueue;
    queue ackQueue;
}connDown;


#endif
