#include "routingEngine.h"

static int shutdownRouter;

void initRouter(routingEngine *router,
                int nodeID,
                int cycleTime,
                int neighborTimeout,
                int retranTimeout,
                int LSATimeout)
{
    router->nodeID = nodeID;
    router->cycleTime = cycleTime;
    router->neighborTimeout = neighborTimeout;
    router->retranTimeout = retranTimeout;
    router->LSATimeout = LSATimeout;
    router->newConnHandler = newConnectionHandler;
    router->readConnHandler = readConnectionHandler;
    router->processConnHandler = processConnectionHandler;
    router->writeConnHandler = writeConnectionHandler;

}

int startRouter(routingEngine *engine)
{
    int routingPort = getRoutingPort(engine->nodeID);
    int localPort = getLocalPort(engine->nodeID);
    int routingFD = openSocket(routingPort, "UDP");
    int localFD = openSocket(localPort, "TCP");
    if(routingFD < 0 || localFD < 0) {
        return EXIT_FAILURE;
    } else {
        printf("UDP Port: %d, TCP Port: %d\n", routingPort, localPort);
    }
    return listenSocket(engine, routingFD, localFD);
}

void exitRouter(routingEngine *engine, DLL *connList)
{
    int i = 0;
    int size = connList->size;
    connObj *connPtr;
    engine = engine;
    for(i = 0; i < size; i++) {
        connPtr = getNodeDataAt(connList, i);
        setConnObjClose(connPtr);
    }
    mapNode(connList);

}

void signalExitRouter()
{
    shutdownRouter = 1;
}

void signalRestartRouter()
{
    shutdownRouter = 2;
}

int listenSocket(routingEngine *engine, int routingFD, int localFD)
{
    int maxSocket = (routingFD > localFD) ? routingFD : localFD;
    int numReady;
    DLL socketList;

    timeval timeout, curTime, oldTime;
    timeout.tv_sec = 1;
    gettimeofday(&oldTime, NULL);
    int advFlag = 0;
    
    fd_set readPool, writePool;
    initList(&socketList, compareConnObj, freeConnObj, mapConnObj);
    insertNode(&socketList, createConnObj(localFD, 0, TCP));
    insertNode(&socketList, createConnObj(routingFD, 0, TCP));
    while(1) {
        if(shutdownRouter != 0) {
            int retVal = shutdownRouter;
            shutdownRouter = 0;
            exitRouter(engine, &socketList);
            return retVal;
        }
        /* Handle time for advertisement event */
        gettimeofday(&curTime, NULL);
        if(curTime.tv_sec - oldTime.tv_sec > engine->cycleTime) {
            advFlag = 1;
        } else {
            advFlag = 0;
        }
        oldTime = curTime;
        if(advFlag == 1) {
            //TODO: implement this
            makeNewAdvertisement();
            continue;
        }
        /* Handle socket event */
        createPool(&socketList, &readPool, &writePool, &maxSocket);
        numReady = select(maxSocket + 1, &readPool, &writePool, NULL, &timeout);
        if(numReady < 0) {
            printf("Select Error\n");
        } else if(numReady == 0 ) {
            printf("Select Idle\n");
        } else {
            handlePool(&socketList, &readPool, &writePool,  engine);
        }
    }
}

void handlePool(DLL *list, fd_set *readPool, fd_set *writePool, routingEngine *engine)
{
    int numPool = list->size;
    if(numPool <= 0) {
        printf( "List error\n");
        return;
    } else {
        int i = 0;
        int listenFd;
        connObj *connPtr;
        /* Handle existing connections */
        printf( "HandlePool: Total Existing [%d]\n", numPool);
        for(i = 2; i < numPool; i++) {
            connPtr = getNodeDataAt(list, i);
            int connFd = getConnObjSocket(connPtr);
            printf( "Existing [%d] ", connFd);
            if(FD_ISSET(connFd, readPool)) {
                printf( "Active RD [%d] ", connFd);
                engine->readConnHandler(connPtr);
                engine->processConnHandler(connPtr);
            }
            if(FD_ISSET(connFd, writePool)) {
                printf( "Active WR [%d] ", connFd);
                engine->writeConnHandler(connPtr);
            }
            printf( "\n");
        }
        /* Accept connection from Flask */
        connPtr = getNodeDataAt(list, 0);
        listenFd = getConnObjSocket(connPtr);
        if(FD_ISSET(listenFd, readPool)) {
            int status = engine->newConnHandler(connPtr);
            if(status > 0) {
                printf( "New Flask connection accpted at [%d]\n", status);
                connPtr = createConnObj(status, BUF_SIZE, TCP);
                insertNode(list, connPtr);
            } else {
                printf("cannot accept new Flask Connection\n");
            }
        }
        /* Handle Routing advertisement on UDP */
        // TODO:Checkpoint 2
        /* Remove closed connections from list */
        mapNode(list);
    }

}

void createPool(DLL *list, fd_set *readPool, fd_set *writePool, int *maxSocket)
{
    int max = -1;
    Node *ref = list->head;
    if(ref == NULL) {
        return;
    } else {
        int i, connFd;
        connObj *connPtr;
        FD_ZERO(readPool);
        FD_ZERO(writePool);
        /* Add listening socket for Flask (TCP) */
        connPtr = getNodeDataAt(list, 0);
        connFd = getConnObjSocket(connPtr);
        FD_SET(connFd, readPool);
        max = connFd;
        printf( "[%dLTCP]", connFd);
        /* Add listening socket for Routing (UDP) */
        connPtr = getNodeDataAt(list, 1);
        connFd = getConnObjSocket(connPtr);
        FD_SET(connFd, readPool);
        max = (connFd > max ) ? connFd : max;
        printf( "[%dUDP]", connFd);
        /* Add client socket */
        i = 2;
        ref = getNodeAt(list, i);
        while(i < list->size) {
            connPtr = ref->data;
            connFd = getConnObjSocket(connPtr);
            printf( "[%d", connFd);
            if(isReadConnObj(connPtr) ) {
                FD_SET(connFd, readPool);
                max = (connFd > max) ? connFd : max;
                printf( "R");
            }
            if(isWriteConnObj(connPtr)) {
                FD_SET(connFd, writePool);
                max = (connFd > max) ? connFd : max;
                printf( "W");
            }
            printf( "]");
            i++;
            ref = ref->next;
        }
        printf( " Max = %d\n", max);
        *maxSocket = max;
    }
}

int openSocket(int port, char *typeStr)
{
    int sock;
    int optval = 1;
    struct sockaddr_in addr;
    int type;
    if(strcmp(typeStr, "TCP") == 0) {
        type = SOCK_STREAM;
    } else {
        type = SOCK_DGRAM;
    }
    if ((sock = socket(PF_INET, type, 0)) == -1) {
        printf("Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    if(setsockopt(sock,
                  SOL_SOCKET,
                  SO_REUSEADDR,
                  (const void *)&optval,
                  sizeof(int)) < 0) {
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports */
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr))) {
        closeSocket(sock);
        printf("Failed binding socket.\n");
        return EXIT_FAILURE;
    }
    if(type == SOCK_STREAM) {
        if (listen(sock, 5) < 0) {
            closeSocket(sock);
            printf("Error listening on socket.\n");
            return EXIT_FAILURE;
        }
    }
    return sock;
}


int closeSocket(int sock)
{
    if (close(sock)) {
        printf("Failed closing socket.\n");
        return 1;
    }
    return 0;
}
