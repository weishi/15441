
#include "selectEngine.h"

void initEngine(selectEngine *engine,
                int port,
                char *logFile,
                int (*newConnHandler)(connObj *),
                void (*readConnHandler)(connObj *),
                void (*writeConnHandler)(connObj *),
                int (*closeConnHandler)(connObj *) )
{

    engine->port = port;
    engine->logFile = logFile;
    engine->newConnHandler = newConnHandler;
    engine->readConnHandler = readConnHandler;
    engine->writeConnHandler = writeConnHandler;
    engine->closeConnHandler = closeConnHandler;
}

int startEngine(selectEngine *engine)
{
    int listenFd = openSocket(engine->port);
    if(listenFd < 0) {
        return EXIT_FAILURE;
    }
    return listenSocket(engine, listenFd);
}

int listenSocket(selectEngine *engine, int listenFd)
{
    int maxSocket = listenFd;
    int numReady;
    DLL socketList;

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    fd_set readPool, writePool;
    initList(&socketList, compareConnObj, freeConnObj, mapConnObj);
    insertNode(&socketList, createConnObj(listenFd, 0));
    while(1) {
        fprintf(stderr, "Selecting...\n");
        createPool(&socketList, &readPool, &writePool, &maxSocket);
        numReady = select(maxSocket + 1, &readPool, &writePool, NULL, NULL);
        if(numReady < 0) {
            fprintf(stderr, "Select Error\n");
        } else if(numReady == 0 ) {
            fprintf(stderr, "Select Idle\n");
        } else {
            handlePool(&socketList, &readPool, &writePool,  engine);
        }
    }
}

void handlePool(DLL *list, fd_set *readPool, fd_set *writePool, selectEngine *engine)
{
    int numPool = list->size;
    if(numPool <= 0) {
        fprintf(stderr, "List error\n");
        return;
    } else {
        int i = 0;
        connObj *connPtr;
        /* Handle existing connections */
        printf("HandlePool: Total Existing [%d]\n", numPool);
        for(i = 1; i < numPool; i++) {
            connPtr = getNodeDataAt(list, i);
            int connFd = getConnObjSocket(connPtr);
            printf("Existing [%d] ", connFd);
            if(FD_ISSET(connFd, readPool)) {
                printf("Active RD [%d] ", connFd);
                engine->readConnHandler(connPtr);
            }
            if(FD_ISSET(connFd, writePool)) {
                printf("Active WR [%d] ", connFd);
                engine->writeConnHandler(connPtr);
            }
            printf("\n");
        }
        /* Accept potential new connection */
        connPtr = getNodeDataAt(list, 0);
        int listenFd = getConnObjSocket(connPtr);
        if(FD_ISSET(listenFd, readPool)) {
            int status = engine->newConnHandler(connPtr);
            if(status > 0) {
                insertNode(list, createConnObj(status, BUF_SIZE));
            } else {
                fprintf(stderr, "cannot accept new Conn\n");
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
        int i;
        FD_ZERO(readPool);
        FD_ZERO(writePool);
        /* Add listening socket */
        connObj *connPtr = getNodeDataAt(list, 0);
        int connFd = getConnObjSocket(connPtr);
        FD_SET(connFd, readPool);
        max = connFd;
        /* Add client socket */
        for(i = 1; i < list->size; i++) {
            connPtr = getNodeDataAt(list, i);
            connFd = getConnObjSocket(connPtr);
            if(!isFullConnObj(connPtr)) {
                FD_SET(connFd, readPool);
                max = (connFd > max) ? connFd : max;
            }
            if(!isEmptyConnObj(connPtr)) {
                FD_SET(connFd, writePool);
                max = (connFd > max) ? connFd : max;
            }
            printf("[%d]", connFd);
        }
        printf(" Max = %d\n", max);
        *maxSocket = max;
    }
}

int openSocket(int port)
{
    int sock;
    int optval = 1;
    struct sockaddr_in addr;
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Failed creating socket.\n");
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
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }
    if (listen(sock, 5) < 0) {
        closeSocket(sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }
    return sock;
}


int closeSocket(int sock)
{
    if (close(sock)) {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

