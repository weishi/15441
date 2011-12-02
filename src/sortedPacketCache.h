#include "packet.h"

typedef struct sortedPacketCache{
  int seq;
  Packet *pkt;
  struct sortedPacketCache* next;
} sortedPacketCache;

/*List-related methods*/
sortedPacketCache * newCache(Packet* pkt, int seq);
void insertInOrder(sortedPacketCache **head, Packet* pkt, int seq);
Packet * removeHead(sortedPacketCache **head);
int flushCache(int expected, queue* queue, sortedPacketCache **cache);
