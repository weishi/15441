#ifndef WINDOW_H
#define WINDOW_H

#include "queue.h"
#include "congestCtrl.h"

#define MAX_DUPLICATE 3

typedef struct sendWindow{
  uint32_t lastPacketAcked;
  uint32_t lastPacketSent;
  uint32_t lastPacketAvailable;
  congestCtrler ctrl;
  uint8_t dupCount;
}sendWindow;

typedef struct recvWindow{
  uint32_t lastPacketRead;
  uint32_t lastPacketRcvd;
  uint32_t nextPacketExpected;
}recvWindow;

void initWindows(recvWindow*, sendWindow*);
void initRecvWindow(recvWindow *);
void initSendWindow(sendWindow *);
void updateRecvWindow(recvWindow *);
void updateSendWindow(sendWindow *);

#endif
