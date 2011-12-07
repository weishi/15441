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

Implemented Slow Start, Congstion Aviodence, Fast Retransmit and Fast recovery.

Generally, SSTHREASh is set to be 64 initially, and congestion avoidence will be triggered when this value is reached. 

-Window size will be reset to 1 upon data loss (triggerred either by timeout or 3 duplicate ACKs).
 SSTHRESH will be set to max(SSTHRESH/2, 2) when this happens

-Receiver does ACK caching when packets with larger-than-expected sequence numbers arrived. 
 Upon receiving the expected packet, the cached ACKs will be flushed and sent back to the sender end as soon as possible.

Failure Tolerence:

Connection will be closed upon 3 connection timeouts. When this happens, 
the sending end will simply clean up the the connection and wait for new requests, and
the receiving end will re-flood the network with WHOHAS packets containing hashes that
have not been downloaded. 

Only one-way connection(either download or upload) is allowed between two particular hosts at one time.



