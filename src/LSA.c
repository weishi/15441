#include "LSA.h"


int compareLSA(void *data1, void *data2)
{
    data1 = data1;
    data2 = data2;
    return 0;
}

void freeLSA(void *data)
{
    LSA *lsa = (LSA *)data;
    freeList(lsa->listObj);
    free(lsa);
}

void *copyLSA(void *data)
{
    LSA *lsa = (LSA *)data;
    return LSAfromLSA(lsa);
}


LSA *newLSA(uint32_t senderID, uint32_t seqNo)
{
    LSA *newObj = malloc(sizeof(LSA));
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
    newObj->hasAck = 0;
    newObj->hasRetran = 0;
    newObj->isDown = 0;
    return newObj;

}
void replaceLSA(LSA **ptr, LSA *nextLSA)
{
    freeLSA(*ptr);
    *ptr = nextLSA;
}

void incLSASeq(LSA *lsa)
{
    lsa->seqNo++;
}

void setLSAAck(LSA *lsa)
{
    lsa->type = 1;
}

int hasLSAAck(LSA *lsa)
{
    return lsa->hasAck;
}

void gotLSAAck(LSA *lsa)
{
    lsa->hasAck = 1;
}

int isLSAAck(LSA *lsa)
{
    return lsa->type == 1;
}

void decLSATTL(LSA *lsa)
{
    lsa->TTL--;
}

int isLSADown(LSA *lsa)
{
    return lsa->isDown;
}

void setLSADown(LSA *lsa)
{
    lsa->TTL = 0;
    lsa->isDown = 1;
}
uint8_t getLSATTL(LSA *lsa)
{
    return lsa->TTL;
}

void setLSADest(LSA *lsa, char *dest, int port)
{
    lsa->destPort = port;
    lsa->dest = dest;
}

LSA *headerLSAfromLSA(LSA *lsa)
{
    LSA *newObj = malloc(sizeof(LSA));
    newObj->src = lsa->src;
    newObj->dest = NULL;
    newObj->srcPort = 0;
    newObj->destPort = 0;
    //Payload
    newObj->version = lsa->version;
    newObj->TTL = lsa->TTL;
    newObj->type = lsa->type;
    newObj->senderID = lsa->senderID;
    newObj->seqNo = lsa->seqNo;
    newObj->numLink = 0;
    newObj->numObj = 0;
    newObj->listLink = NULL;
    newObj->listObj = NULL;
    //Meta
    gettimeofday(&(newObj->timestamp), NULL);
    newObj->hasAck = 0;
    newObj->hasRetran = 0;
    newObj->isDown = 0;
    return newObj;
}

LSA *LSAfromLSA(LSA *lsa)
{
    int size = sizeof(uint32_t) * lsa->numLink;
    LSA *newObj = headerLSAfromLSA(lsa);
    newObj->numLink = lsa->numLink;
    newObj->numObj = lsa->numObj;
    newObj->listLink = malloc(size);
    memcpy(newObj->listLink, lsa->listLink, size);
    newObj->listObj = copyList(lsa->listObj);

    return newObj;
}

LSA *LSAfromBuffer(char *buf, ssize_t length, char *src, int srcPort)
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

    LSA *newObj = malloc(sizeof(LSA));
    newObj->src=src;
    newObj->srcPort=srcPort;
    newObj->dest=NULL;
    newObj->destPort=0;
    //Meta
    gettimeofday(&(newObj->timestamp), NULL);
    newObj->hasAck = 0;
    newObj->hasRetran = 0;
    newObj->isDown = 0;
    //Payload
    newObj->version = version;
    newObj->TTL = TTL;
    newObj->type = type;
    newObj->senderID = senderID;
    newObj->seqNo = seqNo;
    newObj->numLink = numLink;
    newObj->numObj = numObj;
    gettimeofday(&(newObj->timestamp), NULL);
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
    printLSA(newObj);
    return newObj;
}

void printLSA(LSA *lsa){
    printf("==LSA==\n");
    printf("src=%s:%d;dest=%s:%d\n", lsa->src, lsa->srcPort, lsa->dest, lsa->destPort);
    printf("version=%d;TTL=%d;type=%d\n", lsa->version, lsa->TTL, lsa->type);
    printf("senderID=%d;seqNo=%d\n", lsa->senderID, lsa->seqNo);
    printf("numLink=%d;numObj=%d\n", lsa->numLink, lsa->numObj);
    printf("hasAck=%d;hasRetran=%d;isDown=%d\n", lsa->hasAck, lsa->hasRetran, lsa->isDown);

}

void LSAtoBuffer(LSA *lsa, char *buffer, ssize_t *bufSize)
{
    unsigned int i;
    uint16_t val16;
    uint32_t val32;
    ssize_t packetSize;
    char *ptr;
    memcpy(buffer, &(lsa->version), 1);
    memcpy(buffer + 1, &(lsa->TTL), 1);
    val16 = htons(lsa->type);
    memcpy(buffer + 2, &val16, 2);
    val32 = htonl(lsa->senderID);
    memcpy(buffer + 4, &val32, 4);
    val32 = htonl(lsa->seqNo);
    memcpy(buffer + 8, &val32, 4);
    val32 = htonl(lsa->numLink);
    memcpy(buffer + 12, &val32, 4);
    val32 = htonl(lsa->numObj);
    memcpy(buffer + 16, &val32, 4);
    ptr = buffer + 20;
    for(i = 0; i < lsa->numLink; i++) {
        val32 = htonl(lsa->listLink[i]);
        memcpy(ptr + i * 4, &val32, 4);
    }
    ptr += i * 4;

    for(i = 0; i < lsa->numObj; i++) {
        char *objPtr = getNodeDataAt(lsa->listObj, i);
        strcpy(ptr, objPtr);
        ptr += strlen(objPtr) + 1;;
    }
    packetSize = ptr - buffer;
    if(packetSize > *bufSize) {
        printf("Packet too big, %zu / %zu\n", packetSize, *bufSize);
        *bufSize = -1;
    } else {
        printf("Packet written to buffer: %zu bytes\n", packetSize);
        *bufSize = packetSize;
    }
}

void insertLSALink(LSA *lsa, uint32_t nodeID)
{
    uint32_t numLink = lsa->numLink;
    lsa->listLink = realloc(lsa->listLink, sizeof(uint32_t) * (numLink + 1));
    lsa->listLink[numLink] = nodeID;
    lsa->numLink++;
}

void insertLSAObj(LSA *lsa, char *objName)
{
    if(lsa->numObj == 0) {
        lsa->listObj = malloc(sizeof(DLL));
        initList(lsa->listObj, compareString, freeString, NULL, copyString);
    }

    insertNode(lsa->listObj, lsa->listObj->copyData(objName));
    lsa->numObj++;

}


int hasLSARetran(LSA *lsa)
{
    return lsa->hasRetran;
}
void setLSARetran(LSA *lsa)
{
    lsa->hasRetran = 1;
}



