#ifndef HTTPHANDLER_H 
#define HTTPHANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#define CLOSE_ME -1
#define BUF_SIZE 1024


int newConnectionHandler(int);
int oldConnectionHandler(int);
int closeConnectionHandler(int);
#endif
