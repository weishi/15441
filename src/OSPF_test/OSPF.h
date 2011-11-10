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
  unsigned matrixSize; //numNode^2
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
   nodeList[i*col + j] is nodeID for vertex i. So for a 2*2 matrix,
   nodeList = {id_vertex0, id_vertex0, id_vertex1, id_vertex1}
  rootID= the nodeID of the source for shortest path
  
  HAN's CHANGE CHANGES:
  1. pass in number of nodes instead of matrix size. n*n should be cheaper
  than sqrt(n)
  HAN's assumptions:
  1. nodeList[i*col + j] = nodeID for vertex i. So for a 2*2 matrix,
  nodeList = {id_vertex0, id_vertex0, id_vertex1, id_vertex1}
*/

void updateShortestPath(int* matrix, int numNode, unsigned int *nodeList, 
			unsigned int rootID);

/*
  Returns the nodeID of the next hop from root to get to node <targetID>
  following the shortest path. If targetID is root return itself
 */
unsigned int query(unsigned int targetID);
