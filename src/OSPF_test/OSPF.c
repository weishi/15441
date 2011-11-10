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
  int matrixSize= globalState->matrixSize;
  for(i = 0; i < matrixSize; i+= numNode){
    if(nodeList[i] == nodeID)
      return i/numNode;
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
  //removedNodes[i] = 1 means the node i (matrix) is removed 
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
	  if(nodeList[u * numNodes] < nodeList[previous[v] * numNodes])
	    previous[v] = u;
      }
      if(dist[v] < minDist){
	new_u = v;
	minDist = dist[v];
      }
    }
    u = new_u;
  }
}

//assumes caller deals with memory allocation of matrix and nodeList
void updateShortestPath(int *matrix, int numNode, unsigned int *nodeList,
			unsigned int rootID){
  if(!globalState)
    globalState = (shortestPathState*) malloc_clean(sizeof(shortestPathState));

  globalState->matrix = matrix;
  globalState->nodeList = nodeList;
  globalState->numNode = numNode;
  globalState->matrixSize = numNode * numNode;
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

unsigned query(unsigned targetID){
  if(targetID == globalState->rootID)
    return globalState->rootID;
  int *path = globalState->previous;
  int u = fromID(targetID);
  while(path[u] != UNDEFINED){
    if(path[u] == globalState->rootIdx)
      return globalState->nodeList[u * globalState->numNode];
    u = path[u];
  }
  return -1;
}

int main(){
  int matrix1[4][4] = {{0, 1, 1, 0}, //1
		       {1, 0, 0, 1}, //3
		       {1, 0, 0, 1}, //2
		       {0, 1, 1, 0}};//7
  int nodeList1[16] = {1,1,1,1,3,3,3,3,2,2,2,2,7,7,7,7};
  int numNode = 4;
  int rootID = 1;
  updateShortestPath(matrix1[0], numNode, nodeList1, rootID);
  printf("Matrix 1: Next hop from root to 7 is %d\n", query(2));
  
  int matrix2[4][4] = {{0,1,1,0}, //1
		       {1,0,1,0}, //4
		       {1,1,0,1}, //3
		       {0,0,1,0}};//5
  int nodeList2[16] = {1,1,1,1,4,4,4,4,3,3,3,3,5,5,5,5};
  updateShortestPath(matrix2[0], numNode, nodeList2, rootID);
  printf("Matrix 2: Next hop from root to 5 is %d\n", query(5));

  int matrix3[4][4] = {{0,1,1,1}, //1
		       {1,0,1,1}, //4
		       {1,1,0,1}, //3
		       {1,1,1,0}};//5
  int nodeList3[16] = {1,1,1,1,4,4,4,4,3,3,3,3,5,5,5,5};
  updateShortestPath(matrix3[0], numNode, nodeList3, rootID);
  printf("Matrix 2: Next hop from root to 5 is %d\n", query(5));
  
  return 0;
}
