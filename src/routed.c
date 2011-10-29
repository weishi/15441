/*
 *  Routed main entry
 *
 *  Wei Shi <weishi@andrew.cmu.edu>
 *  Han Liu <hanl1@andrew.cmu.edu>
 */

#include "routed.h"

int main(int argc, char *argv[])
{
    int retVal;
    int nodeID;
    char *configFile;
    char *resFile;
    int cycleTime;
    int neighborTimeout;
    int retranTimeout;
    int LSATimeout;
    routingEngine engine;
    if(argc != 8) {
        printf(USAGE, argv[0]);
        return EXIT_FAILURE;
    }

    nodeID = atoi(argv[1]);
    configFile = argv[2];
    resFile = argv[3];
    cycleTime = atoi(argv[4]);
    neighborTimeout = atoi(argv[5]);
    retranTimeout = atoi(argv[6]);
    LSATimeout = atoi(argv[7]);

    /*
    if(daemonize(lockFile)==EXIT_FAILURE){
        printf("Error daemonizing.\n");
        return EXIT_FAILURE;
    }

    if(initLogger(logFile) == -1) {
        printf("Error opening logging file.\n");
        return EXIT_FAILURE;
    }
    */
    printf("%d, %s, %s, %d, %d, %d, %d\n", 
            nodeID, configFile, resFile, 
            cycleTime, neighborTimeout, retranTimeout, LSATimeout);
    
    if(initRoutingTable(nodeID, configFile, resFile) == -1) {
        printf("Error reading router config file.\n");
        return EXIT_FAILURE;
    }
    initRouter(&engine,
               nodeID,
               cycleTime,
               neighborTimeout,
               retranTimeout,
               LSATimeout);

    retVal = startRouter(&engine);
    return retVal;
}
