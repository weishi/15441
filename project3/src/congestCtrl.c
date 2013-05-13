#include "congestCtrl.h"

void triggerSlowStart(congestCtrler *ctrl){
  ctrl->mode = SLOW_START;
}

void triggerCongestAvoid(congestCtrler *ctrl){
  ctrl->mode = CONGEST_AVOID;
}

void initCongestCtrler(congestCtrler *ctrl){
  printf("initing congest ctrl\n");
  ctrl->mode = SLOW_START;
  ctrl->windowSize = 1;
  ctrl->ssthresh = INIT_THRESH;
  ctrl->slowInc = 0;
}

void expandWindow(congestCtrler *ctrl){
  switch(ctrl->mode){
  case SLOW_START:
    ctrl->windowSize++;
    if(ctrl->windowSize == ctrl->ssthresh)
      ctrl->mode = CONGEST_AVOID;
    break;
  case CONGEST_AVOID:
    if(++(ctrl->slowInc) == ctrl->windowSize){
      ctrl->windowSize++;
      ctrl->slowInc = 0;
    }
  }
}

void shrinkWindow(congestCtrler *ctrl){
  ctrl->slowInc = 0;
  ctrl->windowSize = 1;
  ctrl->mode = SLOW_START;
  printf("Shrinking window from %d", ctrl->ssthresh);
  if((ctrl->ssthresh /= 2) < 2)
    ctrl->ssthresh = 2;
  printf("to %d\n", ctrl->ssthresh);
}

/*Logging module*/
void logger(int peerID, uint32_t connID, int timeDif, int windowSize)
{
  if(log_file)
    fprintf(log_file, "f%d%d   %d   %d\n", peerID, connID, timeDif, windowSize);
  else
    fprintf(stderr, "f%d%d\t%d\t%d\n", peerID, connID, timeDif, windowSize);
}
