#include "connPool.h"

void cleanUpConnUp(connUp conn){
  memset(&(conn.sw), 0, sizeof(conn.sw));
  clearQueue(conn.dataQueue);
  clearQueue(conn.ackWaitQueue);
  conn.connected = 0;
  conn.timeoutCount = 0;
}

void cleanUpConnDown(connDown conn){
  memset(&(conn.rw), 0, sizeof(conn.rw));
  conn.state = 0;
  conn.curChunkID = 0;
  conn.connected = 0;
  conn.timeoutCount = 0;
  clearQueue(conn.getQueue);
  clearQueue(conn.timeoutQueue);
  clearQueue(conn.ackSendQueue);
}
