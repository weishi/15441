#ifndef WINDOW_H
#define WINDOW_H

#include "queue.h"

typedef struct sendWindow{
	int lastPacketAcked;
	int lastPacketSent;
	int lastPacketAvailable;
    int windowSize;
}sendWindow;


typedef struct recvWindow{
	int lastPacketRead;
	int lastPacketRcvd;
	int nextPacketExpected;
    int windowSize;
}recvWindow;

void initRecvWindow(recvWindow *);
void initSendWindow(sendWindow *);

#endif
