
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "selectEngine.h"

#define ECHO_PORT 9999
#define BUF_SIZE 4096

#define USAGE "\nUsage: %s <port>\n"
