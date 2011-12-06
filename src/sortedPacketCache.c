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
  
  if(cur == NULL){
    cache->next = cur;
    *head = cache;
    return;
  }
  while(cur != NULL){
    if(cur->next == NULL){
      cur->next = cache;
      break;
    }
    if(seq < cur->next->seq){
      cache->next = cur->next;
      cur->next = cache;
      break;
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
  printf("flushing cache");
  int newExpected = expected + 1;
  if(*cache == NULL)
    return ++expected;
  sortedPacketCache* tmp = *cache;
  while(tmp != NULL){
    printf("Cache seq %d ", tmp->seq);
    tmp = tmp->next;
  }
  printf("\n");
  
  while(*cache != NULL && (*cache)->seq <= newExpected){
    if((*cache)->seq == newExpected)
      newExpected++;
    enqueue(qPtr, removeHead(cache));
  }
  tmp  = *cache;
  while(tmp != NULL){
    printf("After Cache seq %d ", tmp->seq);
    tmp = tmp->next;
  }
  printf("\n");
  printf("old expected %d new expected %d\n", expected, newExpected);
  return newExpected;
}

void clearCache(sortedPacketCache **cache){
  while((*cache) != NULL)
    removeHead(cache);
}

