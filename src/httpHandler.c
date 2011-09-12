
#include "httpHandler.h"

int newConnectionHandler(int socket)
{
    int client_sock=socket;
    ssize_t readret =0;
    char buf[BUF_SIZE];

    while((readret = recv(client_sock, buf, BUF_SIZE, 0)) > 1) {
        if (send(client_sock, buf, readret, 0) != readret) {
            fprintf(stderr, "Error sending to client.\n");
            return EXIT_FAILURE;
        }
        memset(buf, 0, BUF_SIZE);
    }

    if (readret == -1) {
        fprintf(stderr, "Error reading from client socket.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int closeConnectionHandler(int socket)
{
    return EXIT_SUCCESS;
}
