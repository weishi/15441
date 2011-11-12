#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include "OSPF.h"

#define INF 65535
#define UNDEFINED -1

shortestPathState *globalState;

void* malloc_clean(int size){
  void* ret = malloc(size);
  memset(ret, 0, size);
  return ret;
}

unsigned fromID(unsigned nodeID){
  int i;
  int numNode = globalState->numNode;
  int* nodeList = globalState->nodeList;
  for(i = 0; i < numNode; i++){
    if(nodeList[i] == nodeID)
      return i;
  }
  return -1;
}

void doOSPF(){
  int i;
  //u will be the node with the smallest distance in dist[]
  unsigned u = globalState->rootIdx;
  int* dist = globalState->dist;
  int* previous = globalState->previous;
  int* nodeList = globalState->nodeList;
  unsigned numNodes = globalState->numNode;
  //removedNodes[i] = 1 means the node i (in matrix) is removed. This array is used as a small set of integers
  int* removedNodes = (int *)malloc_clean(sizeof(unsigned)*globalState->numNode);
  int numRemaining = globalState->numNode;
  unsigned new_u, v, alt, rowIdx, minDist;
  for(i = 0; i < globalState->numNode; i++){
    dist[i] = INF;
    previous[i] = UNDEFINED;
  }
  dist[globalState->rootIdx] = 0;
  while(numRemaining > 0){
    if(dist[u] == INF)
      break; 
    rowIdx = u*numNodes;
    minDist = INF;
    removedNodes[u] = 1;
    numRemaining--;

    for(v = 0; v < numNodes; v++){
      if(removedNodes[v])
	continue;
      if(globalState->matrix[v + rowIdx] == 1){
	alt = dist[u] + 1;
	if(alt < dist[v]){
	  dist[v] = alt;
	  previous[v] = u;
	} else if(alt == dist[v])
	  if(nodeList[u] < nodeList[previous[v]])
	    previous[v] = u;
      }
      if(dist[v] < minDist){
	new_u = v;
	minDist = dist[v];
      }
    }
    u = new_u;
  }
  free(removedNodes);
}

//assumes caller deals with memory allocation of matrix and nodeList
void updateShortestPath(int *matrix, int numNode, unsigned int *nodeList,
			unsigned int rootID){
  if(!globalState)
    globalState = (shortestPathState*) malloc_clean(sizeof(shortestPathState));

  globalState->matrix = matrix;
  globalState->nodeList = nodeList;
  globalState->numNode = numNode;
  globalState->rootID = rootID;
  globalState->rootIdx = fromID(rootID);
  if(globalState->previous)
    free(globalState->previous);
  if(globalState->dist)
    free(globalState->dist);
  globalState->previous = (int *)malloc_clean(sizeof(unsigned)*numNode);
  globalState->dist = (int *)malloc_clean(sizeof(unsigned)*numNode);
  doOSPF();
}

void query(unsigned targetID, unsigned int* nextHop, int *distance){
  if(targetID == globalState->rootID){
    *nextHop = targetID;
    *distance = 0;
  }
  int dist = 0;
  int *path = globalState->previous;
  int u = fromID(targetID);
  while(path[u] != UNDEFINED){
    dist++;
    if(path[u] == globalState->rootIdx){
      *nextHop = globalState->nodeList[u];
      *distance = dist;
      return;
    }
    u = path[u];
  }
  //did not find a route 
  *distance = INF;
}

int main(){
  unsigned int* nextHop = malloc_clean(sizeof(unsigned));
  int* distance = malloc_clean(sizeof(int));
  int matrix1[4][4] = {{0, 1, 1, 0}, //1
		       {1, 0, 0, 1}, //3
		       {1, 0, 0, 1}, //2
		       {0, 1, 1, 0}};//7 
  int nodeList1[4] = {1,3,2,7};
  int numNode = 4;
  int rootID = 1;
  updateShortestPath(matrix1[0], numNode, nodeList1, rootID);
  query(7, nextHop, distance);
  printf("Matrix 1: Next hop from root to 7 is %d distance %d\n", *nextHop, *distance);
  
  int matrix2[4][4] = {{0,1,1,0}, //1
		       {1,0,1,0}, //4
		       {1,1,0,1}, //3
		       {0,0,1,0}};//5
  int nodeList2[4] = {1,4,3,5};
  updateShortestPath(matrix2[0], numNode, nodeList2, rootID);
  query(5, nextHop, distance);
  printf("Matrix 2: Next hop from root to 5 is %d distance %d\n", *nextHop, *distance);

  int matrix3[4][4] = {{0,1,1,1}, //1
		       {1,0,1,1}, //4
		       {1,1,0,1}, //3
		       {1,1,1,0}};//5
  int nodeList3[4] = {1,4,3,5};
  updateShortestPath(matrix3[0], numNode, nodeList3, rootID);
  query(5, nextHop, distance);
  printf("Matrix 2: Next hop from root to 5 is %d distance %d\n", *nextHop, *distance);
  free(distance);
  free(nextHop);
  return 0;
}
