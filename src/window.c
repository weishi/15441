#include "window.h"

void initRecvWindow(recvWindow *rw)
{
    rw->lastPacketRead = 0;
    rw->lastPacketRcvd = 0;
    rw->nextPacketExpected = 0;
    rw->windowSize = 8;
}

void initSendWindow(sendWindow *sw)
{
    sw->lastPacketAcked = 0;
    sw->lastPacketSent = 0;
    sw->lastPacketAvailable = 0;
    sw->windowSize = 8;
}
