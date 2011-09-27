/*
 *  This file contains the C source code for an echo server.
 *  The server supports concurrent clients with select()
 *  
 *  Wei Shi <weishi@andrew.cmu.edu>
 */

#include "lisod.h"

int main(int argc, char *argv[])
{
    int port;
    selectEngine engine;
    if(argc != 3) {
        printf(USAGE, argv[0]);
        return EXIT_FAILURE;
    }

    port = atoi(argv[1]);
    logFile = argv[2];
    initEngine(&engine,
               port,
               newConnectionHandler,
               readConnectionHandler,
               processConnectionHandler,
               writeConnectionHandler,
               closeConnectionHandler);
    return startEngine(&engine);
}
