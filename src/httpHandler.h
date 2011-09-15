#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

#include "connObj.h"

#define BUF_SIZE 1024


int newConnectionHandler(connObj *);
void readConnectionHandler(connObj *);
void writeConnectionHandler(connObj *);
int closeConnectionHandler(connObj *);
#endif
