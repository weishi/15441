#include "selectEngine.h"

void initEngine(selectEngine *engine,
                int portHTTP,
                int portHTTPS,
                int (*newConnHandler)(connObj *),
                void (*readConnHandler)(connObj *),
                void (*processConnHandler)(connObj *),
                void (*writeConnHandler)(connObj *),
                int (*closeConnHandler)(connObj *),
                char *crtFile,
                char *keyFile )
{

    engine->portHTTP = portHTTP;
    engine->portHTTPS = portHTTPS;
    engine->newConnHandler = newConnHandler;
    engine->readConnHandler = readConnHandler;
    engine->processConnHandler = processConnHandler;
    engine->writeConnHandler = writeConnHandler;
    engine->closeConnHandler = closeConnHandler;
    engine->ctx = initSSL(crtFile, keyFile);
}

int startEngine(selectEngine *engine)
{
    int httpFD = openSocket(engine->portHTTP);
    int httpsFD = openSocket(engine->portHTTPS);
    if(httpFD < 0 || httpsFD < 0) {
        return EXIT_FAILURE;
    }
    return listenSocket(engine, httpFD, httpsFD);
}

int listenSocket(selectEngine *engine, int httpFD, int httpsFD)
{
    int maxSocket = (httpFD > httpsFD) ? httpFD : httpsFD;
    int numReady;
    DLL socketList;

    fd_set readPool, writePool;
    initList(&socketList, compareConnObj, freeConnObj, mapConnObj);
    insertNode(&socketList, createConnObj(httpFD, 0));
    insertNode(&socketList, createConnObj(httpsFD, 0));
    while(1) {
        logger(LogDebug, "Selecting...\n");
        createPool(&socketList, &readPool, &writePool, &maxSocket);
        numReady = select(maxSocket + 1, &readPool, &writePool, NULL, NULL);
        if(numReady < 0) {
            logger(LogProd, "Select Error\n");
        } else if(numReady == 0 ) {
            logger(LogDebug, "Select Idle\n");
        } else {
            handlePool(&socketList, &readPool, &writePool,  engine);
        }
    }
}

void handlePool(DLL *list, fd_set *readPool, fd_set *writePool, selectEngine *engine)
{
    int numPool = list->size;
    if(numPool <= 0) {
        logger(LogDebug, "List error\n");
        return;
    } else {
        int i = 0;
        int listenFd;
        connObj *connPtr;
        /* Handle existing connections */
        logger(LogDebug, "HandlePool: Total Existing [%d]\n", numPool);
        for(i = 2; i < numPool; i++) {
            connPtr = getNodeDataAt(list, i);
            int connFd = getConnObjSocket(connPtr);
            logger(LogDebug, "Existing [%d] ", connFd);
            if(FD_ISSET(connFd, readPool)) {
                logger(LogDebug, "Active RD [%d] ", connFd);
                engine->readConnHandler(connPtr);
            }
            engine->processConnHandler(connPtr);
            if(FD_ISSET(connFd, writePool)) {
                logger(LogDebug, "Active WR [%d] ", connFd);
                engine->writeConnHandler(connPtr);
            }
            logger(LogDebug, "\n");
        }
        /* Accept potential new HTTP connection */
        connPtr = getNodeDataAt(list, 0);
        listenFd = getConnObjSocket(connPtr);
        if(FD_ISSET(listenFd, readPool)) {
            int status = engine->newConnHandler(connPtr);
            if(status > 0) {
                logger(LogDebug, "New HTTP connection accpted\n");
                connPtr = createConnObj(status, BUF_SIZE);
                setConnObjHTTP(connPtr);
                insertNode(list, connPtr);
            } else {
                logger(LogProd, "cannot accept new HTTP Connection\n");
            }
        }
        /* Accept potential new HTTPS connection */
        connPtr = getNodeDataAt(list, 1);
        listenFd = getConnObjSocket(connPtr);
        if(FD_ISSET(listenFd, readPool)) {
            int status = engine->newConnHandler(connPtr);
            if(status > 0) {
                logger(LogDebug, "New HTTPS connection accpted\n");
                connPtr = createConnObj(status, BUF_SIZE);
                setConnObjHTTPS(connPtr, engine->ctx);
                insertNode(list, connPtr);
            } else {
                logger(LogProd, "cannot accept new HTTPS Connection\n");
            }
        }
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
        /* Add HTTP listening socket */
        connPtr = getNodeDataAt(list, 0);
        connFd = getConnObjSocket(connPtr);
        FD_SET(connFd, readPool);
        max = connFd;
        /* Add HTTPS listening socket */
        connPtr = getNodeDataAt(list, 1);
        connFd = getConnObjSocket(connPtr);
        FD_SET(connFd, readPool);
        max = (connFd > max ) ? connFd : max;
        /* Add client socket */
        ref = ref->next;
        i = 1;;
        while(i < list->size) {
            connPtr = ref->data;
            connFd = getConnObjSocket(connPtr);
            logger(LogDebug, "[%d", connFd);
            if(!isFullConnObj(connPtr)) {
                FD_SET(connFd, readPool);
                max = (connFd > max) ? connFd : max;
                logger(LogDebug, "R");
            }
            if(!isEmptyConnObj(connPtr)) {
                FD_SET(connFd, writePool);
                max = (connFd > max) ? connFd : max;
                logger(LogDebug, "W");
            }
            logger(LogDebug, "]");
            i++;
            ref = ref->next;
        }
        logger(LogDebug, " Max = %d\n", max);
        *maxSocket = max;
    }
}

int openSocket(int port)
{
    int sock;
    int optval = 1;
    struct sockaddr_in addr;
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        logger(LogProd, "Failed creating socket.\n");
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
        logger(LogProd, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }
    if (listen(sock, 5) < 0) {
        closeSocket(sock);
        logger(LogProd, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }
    return sock;
}


int closeSocket(int sock)
{
    if (close(sock)) {
        logger(LogProd, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

