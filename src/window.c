#include "window.h"

void initWindows(recvWindow *rw, sendWindow *sw)
{
  initRecvWindow(rw);
  initSendWindow(sw);
}

void initRecvWindow(recvWindow *rw)
{
  rw->lastPacketRead = 0;
  rw->lastPacketRcvd = 0;
  rw->nextPacketExpected = 1;
}

void initSendWindow(sendWindow *sw)
{
  initCongestCtrler(&(sw->ctrl));
  sw->lastPacketAcked = 0;
  sw->lastPacketSent = 0;
  sw->lastPacketAvailable = 7;
  sw->dupCount = 0;
}

void updateRecvWindow(recvWindow *rw)
{
  rw->nextPacketExpected++;
}

void updateSendWindow(sendWindow *sw)
{
  sw->lastPacketAvailable = sw->lastPacketAcked + sw->ctrl.windowSize;
}
