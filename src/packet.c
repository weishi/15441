#include "packet.h"

int comparePacket(void *data1, void *data2)
{
    Packet *pkt1 = data1;
    Packet *pkt2 = data2;
    int isSame = pkt1->senderID == pkt2->senderID &&
                 pkt1->seqNo == pkt2->seqNo &&
                 pkt1->TTL == pkt2->TTL &&
                 pkt1->type == pkt2->type;

    return isSame;
}

void freePacket(void *data)
{
    Packet *pkt = (Packet *)data;
    free(pkt->src);
    free(pkt->dest);
    free(pkt->listLink);
    freeList(pkt->listObj);
    free(pkt);
}

void *copyPacket(void *data)
{
    Packet *pkt = (Packet *)data;
    return PacketfromPacket(pkt);
}


Packet *newPacket(uint32_t senderID, uint32_t seqNo)
{
    Packet *newObj = malloc(sizeof(Packet));
    newObj->src = NULL;
    newObj->dest = NULL;
    newObj->srcPort = 0;
    newObj->destPort = 0;
    //Payload
    newObj->version = 1;
    newObj->TTL = 32;
    newObj->senderID = senderID;
    newObj->seqNo = seqNo;
    newObj->type = 0;
    newObj->numLink = 0;
    newObj->numObj = 0;
    newObj->listLink = NULL;
    newObj->listObj = NULL;
    //Meta
    gettimeofday(&(newObj->timestamp), NULL);
    newObj->hasRetran = 0;
    newObj->isDown = 0;
    newObj->isExpired = 0;
    return newObj;

}

Packet *headerPacketfromPacket(Packet *pkt)
{
    Packet *newObj = malloc(sizeof(Packet));
    if(pkt->src == NULL) {
        newObj->src = NULL;
    } else {
        newObj->src = malloc(strlen(pkt->src) + 1);
        strcpy(newObj->src, pkt->src);
    }
    newObj->dest = NULL;
    newObj->srcPort = pkt->srcPort;
    newObj->destPort = 0;
    //Payload
    newObj->version = pkt->version;
    newObj->TTL = pkt->TTL;
    newObj->type = pkt->type;
    newObj->senderID = pkt->senderID;
    newObj->seqNo = pkt->seqNo;
    newObj->numLink = 0;
    newObj->numObj = 0;
    newObj->listLink = NULL;
    newObj->listObj = NULL;
    //Meta
    gettimeofday(&(newObj->timestamp), NULL);
    newObj->hasRetran = 0;
    newObj->isDown = 0;
    newObj->isExpired = 0;
    return newObj;
}

Packet *PacketfromPacket(Packet *pkt)
{
    int size = sizeof(uint32_t) * pkt->numLink;
    Packet *newObj = headerPacketfromPacket(pkt);
    newObj->numLink = pkt->numLink;
    newObj->numObj = pkt->numObj;
    newObj->listLink = malloc(size);
    memcpy(newObj->listLink, pkt->listLink, size);
    newObj->listObj = copyList(pkt->listObj);

    return newObj;
}

Packet *PacketfromBuffer(char *buf, ssize_t length, char *src, int srcPort)
{
    unsigned int i;
    char *ptr;
    uint8_t version, TTL;
    uint16_t type;
    uint32_t senderID, seqNo, numLink, numObj;
    length = length; //Not used
    version = *(uint8_t *)buf;
    TTL = *(uint8_t *)(buf + 1);
    type = ntohs(*(uint16_t *)(buf + 2));

    senderID = ntohl(*(uint32_t *)(buf + 4));
    seqNo = ntohl(*(uint32_t *)(buf + 8));
    numLink = ntohl(*(uint32_t *)(buf + 12));
    numObj = ntohl(*(uint32_t *)(buf + 16));

    Packet *newObj = malloc(sizeof(Packet));
    newObj->src = src;
    newObj->srcPort = srcPort;
    newObj->dest = NULL;
    newObj->destPort = 0;
    //Meta
    gettimeofday(&(newObj->timestamp), NULL);
    newObj->hasRetran = 0;
    newObj->isDown = 0;
    newObj->isExpired = 0;
    //Payload
    newObj->version = version;
    newObj->TTL = TTL;
    newObj->type = type;
    newObj->senderID = senderID;
    newObj->seqNo = seqNo;
    newObj->numLink = numLink;
    newObj->numObj = numObj;
    /* Get node ID */
    newObj->listLink = malloc(sizeof(uint32_t) * numLink);
    buf = buf + 20;
    for(i = 0; i < numLink; i++) {
        newObj->listLink[i] = ntohl(*(uint32_t *)(buf));
        buf += sizeof(uint32_t);
    }
    /* Get object list */
    newObj->listObj = malloc(sizeof(DLL));
    initList(newObj->listObj, compareString, freeString, NULL, copyString);

    for(i = 0; i < numObj; i++) {
        ptr = calloc(strlen(buf) + 1, 1);
        strcpy(ptr, buf);
        insertNode(newObj->listObj, ptr);
        buf += strlen(buf) + 1;
    }
    printPacket(newObj);
    return newObj;
}

void printPacket(Packet *pkt)
{
    printf("==Packet== ");
    printf("src=%s:%d;dest=%s:%d;", pkt->src, pkt->srcPort, pkt->dest, pkt->destPort);
    printf("version=%d;TTL=%d;type=%d;", pkt->version, pkt->TTL, pkt->type);
    printf("senderID=%d;seqNo=%d;", pkt->senderID, pkt->seqNo);
    printf("numLink=%d;numObj=%d;", pkt->numLink, pkt->numObj);
    printf("hasRetran=%d;isDown=%d", pkt->hasRetran, pkt->isDown);

}

void PackettoBuffer(Packet *pkt, char *buffer, ssize_t *bufSize)
{
    unsigned int i;
    uint16_t val16;
    uint32_t val32;
    ssize_t packetSize;
    char *ptr;
    memcpy(buffer, &(pkt->version), 1);
    memcpy(buffer + 1, &(pkt->TTL), 1);
    val16 = htons(pkt->type);
    memcpy(buffer + 2, &val16, 2);
    val32 = htonl(pkt->senderID);
    memcpy(buffer + 4, &val32, 4);
    val32 = htonl(pkt->seqNo);
    memcpy(buffer + 8, &val32, 4);
    val32 = htonl(pkt->numLink);
    memcpy(buffer + 12, &val32, 4);
    val32 = htonl(pkt->numObj);
    memcpy(buffer + 16, &val32, 4);
    ptr = buffer + 20;
    for(i = 0; i < pkt->numLink; i++) {
        val32 = htonl(pkt->listLink[i]);
        memcpy(ptr + i * 4, &val32, 4);
    }
    ptr += i * 4;

    for(i = 0; i < pkt->numObj; i++) {
        char *objPtr = getNodeDataAt(pkt->listObj, i);
        strcpy(ptr, objPtr);
        ptr += strlen(objPtr) + 1;;
    }
    printPacket(pkt);
    packetSize = ptr - buffer;
    if(packetSize > *bufSize) {
        printf("Packet too big, %zu / %zu\n", packetSize, *bufSize);
        *bufSize = -1;
    } else {
        printf("Size: %zu bytes\n", packetSize);
        *bufSize = packetSize;
    }
}

void insertPacketLink(Packet *pkt, uint32_t nodeID)
{
    uint32_t numLink = pkt->numLink;
    pkt->listLink = realloc(pkt->listLink, sizeof(uint32_t) * (numLink + 1));
    pkt->listLink[numLink] = nodeID;
    pkt->numLink++;
}

void insertPacketObj(Packet *pkt, char *objName)
{
    if(pkt->numObj == 0) {
        pkt->listObj = malloc(sizeof(DLL));
        initList(pkt->listObj, compareString, freeString, NULL, copyString);
    }

    insertNode(pkt->listObj, pkt->listObj->copyData(objName));
    pkt->numObj++;

}





