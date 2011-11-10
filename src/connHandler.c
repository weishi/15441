#include "connHandler.h"

int newConnectionHandler(connObj *connPtr)
{
    struct sockaddr_in clientAddr;
    socklen_t clientLength = sizeof(clientAddr);
    int listenFd = getConnObjSocket(connPtr);
    int newFd = accept(listenFd, (struct sockaddr *)&clientAddr, &clientLength);
    if(newFd == -1) {
        printf("Error accepting socket.\n");
        return -1;
    } else {
        return newFd;
    }
}

void processConnectionHandler(connObj *connPtr)
{
    char *readBuf, *writeBuf;
    ssize_t rSize, wSize;
    int full, retVal;
    if(connPtr->isOpen == 0) {
        printf("Skip connection set to close\n");
        return;
    }
    switch(getConnObjType(connPtr)) {
    case TCP:
        getConnObjReadBufferForRead(connPtr, &readBuf, &rSize);
        getConnObjWriteBufferForWrite(connPtr, &writeBuf, &wSize);
        full = isFullConnObj(connPtr);
        retVal = flaskParse(readBuf, rSize, writeBuf, &wSize, full);
        if(retVal == -1) {
            setConnObjClose(connPtr);
        } else if(retVal == 0) {
            //Wait for next round.
        } else {
            addConnObjWriteSize(connPtr, wSize);
            setConnObjIsWrite(connPtr);
        }
        break;
    case UDP: {
        getConnObjReadBufferForRead(connPtr, &readBuf, &rSize);
        if(rSize > 0) {
            LSA *incomingLSA = LSAfromBuffer(readBuf, rSize);
            removeConnObjReadSize(connPtr, rSize);
            if(newLSA == NULL) {
                printf("Bad LSA packet...Skip\n");
            } else {
                printf("Recv new LSA...Process\n");
                updateRoutingTableFromLSA(incomingLSA);
            }
        }
        getLSAFromRoutingTable(&(connPtr->LSAList));
        break;
    }
    default:
        break;
    }
    return;
}

void readConnectionHandler(connObj *connPtr)
{
    if(!isFullConnObj(connPtr)) {
        char *buf;
        ssize_t size;
        int connFd = getConnObjSocket(connPtr);
        ssize_t retSize = 0;
        getConnObjReadBufferForWrite(connPtr, &buf, &size);
        switch(getConnObjType(connPtr)) {
        case TCP:
            printf("Flask client...\n");
            retSize = recv(connFd, buf, size, 0);
            break;
        case UDP: {
            char *ip;
            struct sockaddr_in client;
            unsigned int clientLen = sizeof(client);
            printf("Peer routers...\n");
            retSize = recvfrom(connFd, buf, size, 0,
                               (struct sockaddr *)&client, &clientLen);
            ip = inet_ntoa(client.sin_addr);
            connPtr->src = malloc(strlen(ip) + 1);
            strcpy(connPtr->src, ip);
            break;
        }
        default:
            retSize = -1 ;
        }
        if (retSize == -1) {
            printf("Error reading from client.\n");
            if(errno == EINTR) {
                printf("RECV EINTR. Try later again.\n");
                return;
            }
            setConnObjClose(connPtr);
            return;
        } else if(retSize == 0) {
            printf("Client Closed [%d]", connFd);
            setConnObjClose(connPtr);
            return;
        } else {
            printf("Read %zu bytes\n", retSize);
            addConnObjReadSize(connPtr, retSize);
        }
    }
}


void writeConnectionHandler(connObj *connPtr)
{
    char *buf;
    ssize_t size, retSize;
    int connFd = getConnObjSocket(connPtr);
    switch(getConnObjType(connPtr)) {
    case TCP:
        getConnObjWriteBufferForRead(connPtr, &buf, &size);
        printf("Ready to write %zu bytes...", size);
        printf("Sending to Flask");
        retSize = send(connFd, buf, size, 0);
        break;
    case UDP: {
        //Write as many as possible, until block
        printf("Sending to Peer\n");
        struct sockaddr_in dest;
        DLL *list = getConnObjLSAList(connPtr);
        getConnObjWriteBufferForWrite(connPtr, &buf, &size);
        printf("Write buffer has %zu bytes free\n", size);
        printf("Has %d UDP in queue\n", list->size);
        while(list->size > 0) {
            LSA *thisLSA = getNodeDataAt(list, 0);
            LSAtoBuffer(thisLSA, buf, &size);
            printf("Ready to write %zu bytes of UDP to %s:%d...\n", 
                    size, thisLSA->dest, thisLSA->port);
            //Prepare destination address/port
            memset(&dest, '\0', sizeof(dest));
            dest.sin_family = AF_INET;
            dest.sin_addr.s_addr = inet_addr(thisLSA->dest);
            dest.sin_port = htons(thisLSA->port);
            retSize = sendto(connFd, buf, size, 0,
                             (struct sockaddr *)&dest, sizeof(dest));
            if(retSize == -1) {
                break;
            } else {
                printf("%d LSA left.\n", list->size);
                removeNodeAt(list, 0);
                removeConnObjWriteSize(connPtr, size);
            }
        }
        break;
    }
    default:
        retSize = -1;
    }
    if(retSize == -1) {
        if(errno == EINTR) {
            printf("RECV EINTR. Try later again.\n");
        } else if(errno == EAGAIN || errno == EWOULDBLOCK) {
            printf("UDP Write would block. Wait for next round.\n");
        } else {
            printf("Error sending to client. Close connection.\n");
            setConnObjClose(connPtr);
        }
    } else if(getConnObjType(connPtr) == TCP && retSize != size) {
        printf("Sending to client with short count.\n");
        removeConnObjWriteSize(connPtr, retSize);
    } else if(getConnObjType(connPtr) == TCP) {
        printf("WriteBuf cleared. Close connection.\n");
        removeConnObjWriteSize(connPtr, size);
        setConnObjClose(connPtr);
    } else {
        printf("All Buffered LSA sent.\n");
    }

}

