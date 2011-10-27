#ifndef FLASKPARSER_H
#define FLASKPARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "resourceTable.h"
#include "routingTable.h"

int flaskParse(char *, ssize_t , char , ssize_t , int );

int flaskGETResponse(char *, char *, ssize_t );
int flaskADDResponse(char *, char *, char *, ssize_t );
#endif
