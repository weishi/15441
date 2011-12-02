#include "sortedPacketCache.h"
#include "queue.h"

sortedPacketCache *newCache(Packet* pkt, int seq){
  sortedPacketCache* cache = malloc(sizeof(sortedPacketCache));
  memset(cache, 0, sizeof(sortedPacketCache));
  cache->seq = seq;
  cache->pkt = pkt;
  cache->next = NULL;
  return cache;
}

void insertInOrder(sortedPacketCache **head, Packet* pkt, int seq){
  sortedPacketCache* cache = newCache(pkt, seq);
  sortedPacketCache* cur = *head;
  
  if(cur == NULL || seq < cur->seq){
    cache->next = cur;
    *head = cache;
    return;
  }
  while(cur != NULL){
    if(seq > cur->seq){
      cache->next = cur->next;
      cur->next = cache;
      return;
    }
    cur = cur->next;
  }
}

Packet *removeHead(sortedPacketCache **head){
  sortedPacketCache *tmp = *head;
  Packet* ret;
  *head = tmp->next;
  ret = tmp->pkt;
  free(tmp);
  return ret;
}

int flushCache(int expected, queue* qPtr, sortedPacketCache** cache){
  int newExpected = expected;
  //printf("%p %d %p", cache, cache->seq, cache->next);
  if(*cache == NULL)
    return ++expected;
  while((*cache)->seq == ++newExpected){
    enqueue(qPtr, removeHead(cache));
  }
  printf("old expected %d new expected %d\n", expected, newExpected);
  return newExpected;
}

