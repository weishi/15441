Bittorrent client implementation

+ Components

The client is devided into the following components: 

peer: handles the incoming connection at listening 
    socket and create socket for new connection.
Packet: Creates and handles various types of Packet
Connection Pool: A pool that keeps track of upload/download
    connection between peers
Queue: a queue implementation, used to track packet buffer
SortedPacketQueue: a queue to store outstanding packets in order
    based on their sequence number

Congestion Control:

