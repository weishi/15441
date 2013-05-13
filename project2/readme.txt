Distributed HTTP server implementation

+ Components

The routing daemon is devided into the following components: 

Routing engine: handles the incoming connection at listening 
    socket and create socket for new connection.
OSPF: implemented as Dijkstra's shortest path algorithm. RoutingTable
    will pass in the full matrix representing the topology before it
    queries for next hop.
Routing table:
    store my and peer's information from configure file and advertisement.
    Also handles all incoming LSA and operates based on LSA type
Resource table:
    store my and peer's file information. 
Flask handler:
    parse data from buffer and build up a request object
Socket container: an abstraction model, which contains socket
     status and provides socket operation.
Connection handler: provides an interface between high level parser and low level 
    socket read/write
Linked list: a linked list implementation, used to track
     active socketsi, and header key/value pair.

+ Algorithm

Parser:
If buffer is not empty, do
    Look for "GETRD" or "ADDFILE"
    Parse the length
    Read data upto length bytes
    Throw away the rest, if any.

OSPF:
    Dijkstra algorithm 
