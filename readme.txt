HTTP/1.1 + CGI server implementation

+ Components

The server is devided into the following components: 

Select engine: handles the incoming connection at listening 
    socket and create socket for new connection.
Http handler:
    1) Http Parser: parse data from buffer and build up a request object
    2) Http Responder: take request object and build up a response object
        It builds regular HTTP responses as well as CGI responses, depending on
        the URI. 
Socket container: an abstraction model, which contains socket
     status and provides socket operation.
Connection handler: provides an interface between high level parser and low level 
    socket read/write
Linked list: a linked list implementation, used to track
     active socketsi, and header key/value pair.
Common lib: supplement to c string lib
FileIO lib: handle will file related operation, eg open, write, get metadata
SSL lib: initial and tear down SSL context and connections.

+ Algorithm

Select:
Add listening socket into select's READ set;
for each select() return, do
    if existing socket is ready to read, read data into buffer
    if existing socket is ready to write, write buffer to socket
    if listening socket is ready to read, accept new connection.
    Add new connection into READ set
    If existing socket's buffer is not full, add it to READ set
    If existing socket's buffer is not empty, add it to WRITE set
    If existing socket has closed, remove it from list.
    
Parser:
If buffer is not empty, do
    If parsing Headers
        Look for next "\r\n"
        Parse current pointer up to "\r\n"
        Remove parsed line
        Shift lines after that to make room for more requests
    If parsing Contents
        Take the whole buffer
        Append correct length of data, depending on Content-length and length read so far

SSL:
Use standard TLS algorithm in OpenSSL library to support HTTPS.
 
