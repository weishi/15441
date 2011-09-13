
#include "selectEngine.h"

void initEngine(selectEngine *engine,
                int port,
                char *logFile,
                int (*newConnHandler)(int),
                int (*oldConnHandler)(int),
                int (*closeConnHandler)(int) )
{

    engine->port = port;
    engine->logFile = logFile;
    engine->newConnHandler = newConnHandler;
    engine->oldConnHandler = oldConnHandler;
    engine->closeConnHandler = closeConnHandler;
}

int startEngine(selectEngine engine)
{
    int listenFd = openSocket(engine.port);
    if(listenFd < 0) {
        return EXIT_FAILURE;
    }
    return listenSocket(engine, listenFd);
}

int listenSocket(selectEngine engine, int listenFd)
{
    int maxSocket = listenFd;
    int numReady;
    DLL socketList;
    fd_set pool;
    initList(&socketList, compareInt, freeInt);
    insertNode(&socketList, (void *)((intptr_t)listenFd));
    while(1) {
        fprintf(stderr, "Selecting...\n");
        createPool(socketList, &pool, &maxSocket);
        numReady = select(maxSocket + 1, &pool, NULL, NULL, NULL);
        if(numReady < 0) {
            fprintf(stderr, "Select Error\n");
            return EXIT_FAILURE;
        } else if(numReady == 0 ) {
            fprintf(stdout, "Select Idle\n");
        } else {
            handlePool(&socketList, &pool, engine);
        }
    }
}

void handlePool(DLL *list, fd_set *pool, selectEngine engine)
{
    int numPool = list->size;
    int *closedPool = malloc(numPool * sizeof(int));
    if(numPool <= 0) {
        fprintf(stderr, "List error\n");
        return;
    } else {
        int i = 0;
        int newSocket = -1;
        int numClosed = 0;
        int listenfd = (intptr_t)getNodeDataAt(list, 0);
        /* Accept potential new connection */
        if(FD_ISSET(listenfd, pool)) {
            int status = engine.newConnHandler(listenfd);
            if(status >= 0) {
                newSocket = status;
            } else {
                fprintf(stderr, "cannot accept new Conn\n");
            }
        }
        /* Handle existing connection */
        for(i = 1; i < numPool; i++) {
            int fd = (intptr_t)getNodeDataAt(list, i);
            if(FD_ISSET(fd, pool)) {
                int status = engine.oldConnHandler(fd);
                if(status == CLOSE_ME) {
                    closedPool[numClosed++] = i;
                }
            }
        }
        /* Remove closed connection from pool */
        for(i = 0; i < numClosed; i++) {
            int closeFd=closedPool[i];
            removeNodeAt(list,closeFd);
            closeSocket(closeFd);
        }
        /* Add new connection to pool */
        if(newSocket >= 0) {
            insertNode(list, (void *)((intptr_t)newSocket));
        }
        free(closedPool);
    }

}

void createPool(DLL list, fd_set *pool, int *maxSocket)
{
    int max = -1;
    Node *ref = list.head;
    if(ref == NULL) {
        return;
    } else {
        FD_ZERO(pool);
        do {
            int fd = (intptr_t)ref->data;
            FD_SET(fd, pool);
            max = (fd > max) ? fd : max;
            ref = ref->next;
        } while(ref != NULL);
    }
    *maxSocket = max;
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

