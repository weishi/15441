/*
 * Wei Shi <weishi@andrew.cmu.edu>
 */

#include "zmqBridge.h"


int main(int argc, char **argv)
{
    if(argc != 3) {
        printf("Usage: zmq_bridge <port 1> <port 2>\n");
        exit(EXIT_FAILURE);
    }
    int ret;
    int port1, port2;
    char portBuf[128];
    void *context = zmq_init(1);
    void *pubber = zmq_socket(context, ZMQ_PUB);
    void *subber = zmq_socket(context, ZMQ_SUB);
    port1 = atoi(argv[1]);
    port2 = atoi(argv[2]);
    printf("%d-->%d\n", port1, port2);

    bzero(portBuf, 128);
    sprintf(portBuf, "tcp://*:%d", port1);
    printf("Subber: %s\n", portBuf);
    ret = zmq_bind(subber, portBuf);
    if(ret < 0) {
        printf("Error no= %s\n", strerror(errno));
    }
    zmq_setsockopt(subber, ZMQ_SUBSCRIBE, "", 0);

    bzero(portBuf, 128);
    sprintf(portBuf, "tcp://*:%d", port2);
    printf("Pubber: %s\n", portBuf);
    ret = zmq_bind(pubber, portBuf);
    if(ret < 0) {
        printf("Error no= %s\n", strerror(errno));
    }


    subq = newqueue();
    pubq = newqueue();

    /*Init sockaddr for UDP*/
    memset((char *)&client, 0, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(INT_PORT2);
    inet_aton("127.0.0.1", &client.sin_addr);

    memset((char *)&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(INT_PORT1);
    inet_aton("127.0.0.1", &server.sin_addr);

    startBridge(subber, pubber);
    //Clean up;
    clearQueue(subq);
    clearQueue(pubq);
    //Clean zmq
    return EXIT_SUCCESS;
}

void startBridge(void *subber, void *pubber)
{
    int sendUDP = openSocket(INT_PORT1);
    int recvUDP = openSocket(INT_PORT2);
    int ret;
    int test;
    zmq_pollitem_t pool[4];
    while(1) {
        /* 0=SUB, 1=PUB*/
        pool[0].socket = subber;
        pool[0].events = ZMQ_POLLIN;
        pool[1].socket = pubber;
        if(peek(pubq) != NULL) {
            pool[1].events = ZMQ_POLLOUT;
        } else {
            pool[1].events = 0;
        }
        /* 2=OUT, 3=IN */
        pool[2].socket = NULL;
        pool[2].fd = sendUDP;
        pool[2].events = 0;

        test = outDC != NULL || (subq != NULL && peek(subq) != NULL);
        test = test && waitACK == NULL;
        if(test) {
            pool[2].events |= ZMQ_POLLOUT;
        }
        if(waitACK != NULL) {
            pool[2].events |= ZMQ_POLLIN;
        }

        pool[3].socket = NULL;
        pool[3].fd = recvUDP;
        pool[3].events = ZMQ_POLLIN;
        if(outACK != NULL) {
            pool[3].events |= ZMQ_POLLOUT;
        }
        printf("Polling...");
        ret = zmq_poll(pool, 4, -1);
        printf(" %d\n", ret);
        if(ret > 0) {
            if(pool[0].revents == ZMQ_POLLIN) {
                printf("handleSUB\n");
                handleSUB(subber);
            }
            if(pool[1].revents == ZMQ_POLLOUT) {
                printf("handlePUB\n");
                handlePUB(pubber);
            }
            if(((pool[2].revents) & (ZMQ_POLLOUT)) == ZMQ_POLLOUT) {
                printf("handleSData\n");
                handleServerData(sendUDP);
            }
            if(((pool[2].revents) & (ZMQ_POLLIN)) == ZMQ_POLLIN) {
                printf("handleSAck\n");
                handleServerACK(sendUDP);
            }
            if(((pool[3].revents) & (ZMQ_POLLIN)) == ZMQ_POLLIN) {
                printf("handleCData\n");
                handleClientData(recvUDP);
            }
            if(((pool[3].revents) & (ZMQ_POLLOUT)) == ZMQ_POLLOUT) {
                printf("handleCAck\n");
                handleClientACK(recvUDP);
            }

        }
    }

}

void handleSUB(void *subber)
{
    zmq_msg_t msg;
    int rc = zmq_msg_init(&msg);
    int size;
    dataChunk *dc = (dataChunk *)malloc(sizeof(dataChunk));
    assert(rc == 0);
    rc = zmq_recv(subber, &msg, 0);
    assert(rc >= 0);
    size = zmq_msg_size(&msg);
    dc->size = size;
    dc->curSize = 0;
    dc->data = (char *)malloc(size);
    memcpy(dc->data, zmq_msg_data(&msg), size);
    zmq_msg_close(&msg);
    printf("Msg[Sub]: size=%d\n", dc->size);
    enqueue(subq, dc);
}

void handlePUB(void *pubber)
{
    dataChunk *dc = (dataChunk *)dequeue(pubq);
    if(dc == NULL) {
        printf("nothing to send\n");
        return;
    }
    zmq_msg_t msg;
    zmq_msg_init_size(&msg, dc->size);
    memcpy(zmq_msg_data(&msg), dc->data, dc->size);
    if(zmq_send(pubber, &msg, 0) < 0) {
        printf("Fail sending to PUB\n");
    }
    zmq_msg_close(&msg);
    freeDataChunk(dc);
}

void handleServerData(int sock)
{
    if(outDC == NULL) {
        dataChunk *dc = (dataChunk *)dequeue(subq);
        if(dc == NULL) {
            printf("nothing to send to UDP\n");
            return;
        }
        outDC = newqueue();
        chunkifyDC(outDC, dc);
        freeDataChunk(dc);
    }
    if(waitACK == NULL) {
        dataPacket *pkt = (dataPacket *)dequeue(outDC);
        if(pkt == NULL) {
            printf("No more pkt to send to UPD\n");
            free(outDC);
            outDC = NULL;
        } else {
            char buf[1500];
            bzero(buf, 1500);
            memcpy(buf, &pkt->id, 4);
            memcpy(buf + 4, &pkt->size, 4);
            memcpy(buf + 8, &pkt->allSize, 4);
            memcpy(buf + 12, &pkt->ack, 4);
            memcpy(buf + 16, &pkt->data, pkt->size);
            sendto(sock, buf, 1500, 0, (struct sockaddr *)&client, sizeof(client));
            waitACK = pkt;
        }
    }


}

void handleServerACK(int sock)
{

    if(waitACK == NULL) {
        printf("ACK Out of sync\n");
        return;
    }
    struct sockaddr_in other;
    char buf[1500];
    bzero(buf, 1500);
    bzero(&other, sizeof(other));
    socklen_t len = 0;;
    int retVal = recvfrom(sock, buf, 1500, 0, (struct sockaddr *)&other, &len);
    if(retVal == -1) {
        printf("Error recv ACK on server side\n");
    } else {
        uint32_t *id = (uint32_t *)buf;
        uint32_t *ack = (uint32_t *)(buf + 12);
        if(*id == waitACK->id && *ack == 1) {
            printf("GOt ACK on server side\n");
            free(waitACK);
            waitACK = NULL;
        } else {
            printf("Recv ill-formed ACK on server side\n");
        }
    }
}

void handleClientData(int sock)
{
    struct sockaddr_in other;
    bzero(&other, sizeof(other));
    char buf[1500];
    bzero(buf, 1500);
    socklen_t len = 0;
    int retVal = recvfrom(sock, buf, 1500, 0,
                          (struct sockaddr *)&other, &len);
    if(retVal == -1) {
        printf("Error recv DATA on client side\n");
    } else {
        uint32_t *id = (uint32_t *)buf;
        uint32_t *size = (uint32_t *)(buf + 4);
        uint32_t *allSize = (uint32_t *)(buf + 8);
        if(inDC == NULL) {
            inDC = (dataChunk *)malloc(sizeof(dataChunk));
            inDC->data = (char *)malloc(*allSize);
            inDC->size = *allSize;
            inDC->curSize = 0;
        }
        memcpy(inDC->data + inDC->curSize, buf + 16, *size);
        inDC->curSize = inDC->curSize+ *size;
        printf("Pktsize=%d,id=%d,allSize=%d|DC:curSize=%d, allSize=%d\n",
                *size, *id, *allSize, inDC->curSize, inDC->size);
        free(outACK);
        outACK = (dataPacket *)malloc(sizeof(dataPacket));
        outACK->id = *id;
        outACK->size = 0;
        outACK->allSize = 0;
        outACK->ack = 1;
        bzero(outACK->data, DATASIZE);
        /* Check if dataChunk is complete*/
        if(inDC->curSize == inDC->size) {
            enqueue(pubq, inDC);
            inDC = NULL;
        }
    }
}

void handleClientACK(int sock)
{
    if(outACK == NULL) {
        return;
    }
    dataPacket *pkt = outACK;
    char buf[1500];
    bzero(buf, 1500);
    memcpy(buf, &pkt->id, 4);
    memcpy(buf + 4, &pkt->size, 4);
    memcpy(buf + 8, &pkt->allSize, 4);
    memcpy(buf + 12, &pkt->ack, 4);
    sendto(sock, buf, 1500, 0, (struct sockaddr *)&server, sizeof(server));
    free(outACK);
    outACK = NULL;
}

void chunkifyDC(queue *q, dataChunk *dc)
{
    int size = dc->size;
    int numChunk = size / DATASIZE;
    int i;
    char *dataPtr = dc->data;
    dataPacket *pkt;
    int lastSize = size % DATASIZE;
    if(lastSize > 0) {
        numChunk++;
    }
    printf("%d chunks\n", numChunk);
    for(i = 0; i < numChunk; i++) {
        pkt = (dataPacket *)malloc(sizeof(dataPacket));
        pkt->id = i;
        pkt->allSize = size;
        pkt->ack = 0;
        if(i == numChunk - 1 && lastSize > 0 ) {
            memcpy(pkt->data, dataPtr + i * DATASIZE, lastSize);
            pkt->size = lastSize;
        } else {
            memcpy(pkt->data, dataPtr + i * DATASIZE, DATASIZE);
            pkt->size = DATASIZE;
        }
        enqueue(q, (void *)pkt);
    }
}

void freeDataChunk(dataChunk *dc)
{
    if(dc != NULL) {
        free(dc->data);
        free(dc);
    }
}

int openSocket(int port )
{
    int sock;
    int optval = 1;
    struct sockaddr_in addr;
    int type = SOCK_DGRAM;
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
        close(sock);
        printf("Failed binding socket.\n");
        return EXIT_FAILURE;
    }
    return sock;
}
