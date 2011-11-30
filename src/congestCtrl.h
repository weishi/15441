#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#define SLOW_START 0
#define CONGEST_AVOID 1

typedef struct congestCtrler {
  uint32_t windowSize;
  uint32_t ssthresh;
  uint32_t slowInc; //used to implement congestion avoidence
  uint8_t mode;
} congestCtrler;

FILE * log_file;

void triggerSlowStart(congestCtrler *);
void triggerCongestAvoid(congestCtrler *);
void initCongestCtrler(congestCtrler *);
void expandWindow(congestCtrler *);
void shrinkWindow(congestCtrler *);
void logger(int, uint32_t, int, int);
