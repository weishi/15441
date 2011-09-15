/******************************************************************************
* lisod.c                                                                     *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server supports concurrent clients with select()               *
*                                                                             *
* Authors: Wei Shi <weishi@andrew.cmu.edu>                                    *
*                                                                             *
*******************************************************************************/

#include "lisod.h"

int main(int argc, char *argv[])
{
    int port;
    char *logFile;
    selectEngine engine;
    if(argc != 3) {
        printf(USAGE, argv[0]);
        return EXIT_FAILURE;
    }

    port = atoi(argv[1]);
    logFile=argv[2];
    initEngine(&engine,
               port,
               logFile,
               newConnectionHandler,
               readConnectionHandler,
               writeConnectionHandler,
               closeConnectionHandler);
    fprintf(stdout, "----- Echo Server -----\n");
    return startEngine(&engine);
}
