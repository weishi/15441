#ifndef HTTPHANDLER_H 
#define HTTPHANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

int newConnectionHandler(int);
int closeConnectionHandler(int);

#endif
