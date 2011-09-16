Echo server implementation

+ Components

The server is devided into the following components: 

Select engine: handles the incoming connection at listening 
    socket and create socket for new connection.
Http handler: read and write data from socket. This does not
     parse http request yet, but only echo data.
Socket container: an abstraction model, which contains socket
     status and provides socket operation.
Linked list: a linked list implementation, used to track
     active sockets.

+ Algorithm

Add listening socket into select's READ set;
for each select() return, do
    if existing socket is ready to read, read data into buffer
    if existing socket is ready to write, write buffer to socket
    if listening socket is ready to read, accept new connection.
    Add new connection into READ set
    If existing socket's buffer is not full, add it to READ set
    If existing socket's buffer is not empty, add it to WRITE set
    If existing socket has closed, remove it from list.
    
    
 
