#include "connPool.h"

void cleanUpConnUp(connUp *conn){
  initSendWindow(&(conn->sw));
  clearQueue(conn->dataQueue);
  clearQueue(conn->ackWaitQueue);
  conn->timeoutCount = 0;
}

void cleanUpConnDown(connDown *conn){
  initRecvWindow(&(conn->rw));
  conn->state = 0;
  conn->curChunkID = 0;
  conn->connected = 0;
  conn->timeoutCount = 0;
  clearQueue(conn->getQueue);
  clearQueue(conn->timeoutQueue);
  clearQueue(conn->ackSendQueue);
}
