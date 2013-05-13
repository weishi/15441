#ifndef DAEMONIZE_H
#define DAEMONIZE_H

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "selectEngine.h"

void signal_handler(int);

int daemonize(char *);

#endif
