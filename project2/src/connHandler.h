#ifndef CONNHANDLER_H
#define CONNHANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "connObj.h"
#include "flaskParser.h"

#define BUF_SIZE 8192 


int newConnectionHandler(connObj *);
void readConnectionHandler(connObj *);
void processConnectionHandler(connObj *);
void writeConnectionHandler(connObj *);

#endif
