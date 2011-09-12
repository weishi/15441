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

int main(int argc, char* argv[])
{
    int port;
    selectEngine engine;
    if(argc < 3) {
        printf(USAGE,argv[0]);
        return EXIT_FAILURE;
    }

    port = atoi(argv[1]);
    initEngine(&engine, port, NULL, NULL, NULL);
    fprintf(stdout, "----- Echo Server -----\n");
    return startEngine(&engine);
}
