/*OSPF.h
  Implemented with Dijkstra
*/

//Structure that stores state of the graph
typedef struct shortestPathState{
  int *matrix;
  int *nodeList;
  int *previous; 
  int *dist;
  unsigned numNode;
  unsigned rootID;
  unsigned rootIdx; //Index of the rootNode within the connection matrix
} shortestPathState;


//returnes the local row index in the matrix given a nodeID
unsigned fromID(unsigned nodeID);

/*Update the shortestPathState using Dijstra's algorithm
  
  Parameters:
  matrix:1-D array connection matrix representing the 2-D graph matrix, 
   where matrix[i*numCol + j] = 1 means node i and j are connected 
   matrix should always be a square matrix
  numNode:
   number of nodes in the graph
  nodeList: map from local matrix indeces to actualy node IDs.
   nodeList[i] is nodeID for vertex i.
  rootID= the nodeID of the source for shortest path
*/  
 
void updateShortestPath(int* matrix, int numNode, unsigned int *nodeList, 
			unsigned int rootID);

/*
  Stores the next hop ID in address pointed by nextHop, and distance to target node
  in address pointed by distance. In case there does not exist a route, distance will become
  INF. 
*/
void query(unsigned int targetID, unsigned int* nextHop, int* distance);
