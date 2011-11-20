#ifndef WINDOW_H
#define WINDOW_H

#include "queue.h"

typedef struct sendWindow{
	int LastPacketAcked;
	int LastPacketSent;
	int LastPacketAvailable;
	queue sQueue;
}sendWindow;


typedef struct recvWindow{
	int LastPacketRead;
	int LastPacketRcvd;
	int NextPacketExpected;
	queue rQueue;
}recvWindow;


#endif
