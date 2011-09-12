
#include "selectEngine.h"

void initEngine(selectEngine* engine,
                int port,
                char* logFile,
                int (*newConnHandler)(int),
                int (*closeConnHandler)(int) )
{

    engine->port=port;
    engine->logFile=logFile;
    engine->newConnHandler=newConnHandler;
    engine->closeConnHandler=closeConnHandler;
}

int startEngine(selectEngine* engine)
{
    int listenFd=openSocket(engine->port);
    if(listenFd <0) {
        return EXIT_FAILURE;
    }
    listenSocket(engine, listenFd);
    return EXIT_SUCCESS;
}

void listenSocket(selectEngine* engine, int listenFd)
{
    DLL socketList;
    initList(&socketList, compareInt, freeInt);
    insertNode(&socketList, (intptr_t *)listenFd);

}

int openSocket(int port)
{
    int sock;
    int optval=1;
    struct sockaddr_in addr;
    /* all networked programs must create a socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(const void *)&optval, sizeof(int)) < 0) {
        return EXIT_FAILURE;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr))) {
        closeSocket(sock);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }
    if (listen(sock, 5)<0) {
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

