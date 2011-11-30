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
  ctrl->ssthresh = 64;
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
  if((ctrl->ssthresh /= 2) < 2)
    ctrl->ssthresh = 2;
}

/*Logging module*/
void logger(int peerID, uint32_t connID, int timeDif, int windowSize)
{
  if(log_file)
    fprintf(log_file, "%d%d   %d   %d\n", peerID, connID, timeDif, windowSize);
  else
    fprintf(stderr, "%d%d   %d   %d\n", peerID, connID, timeDif, windowSize);
  //fflush(log_file);
}
