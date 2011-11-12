#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include "../OSPF.h"

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
