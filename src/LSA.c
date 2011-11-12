#include "LSA.h"

int compareLSA(void *data1, void *data2)
{
    LSA *lsa1 = data1;
    LSA *lsa2 = data2;
    int isSame = lsa1->senderID == lsa2->senderID &&
                 lsa1->seqNo == lsa2->seqNo &&
                 lsa1->TTL == lsa2->TTL &&
                 lsa1->type == lsa2->type;

    return isSame;
}

void freeLSA(void *data)
{
    LSA *lsa = (LSA *)data;
    free(lsa->src);
    free(lsa->dest);
    free(lsa->listLink);
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
    lsa->dest = malloc(strlen(dest) + 1);
    strcpy(lsa->dest, dest);
}

LSA *headerLSAfromLSA(LSA *lsa)
{
    LSA *newObj = malloc(sizeof(LSA));
    if(lsa->src == NULL) {
        newObj->src = NULL;
    } else {
        newObj->src = malloc(strlen(lsa->src) + 1);
        strcpy(newObj->src, lsa->src);
    }
    newObj->dest = NULL;
    newObj->srcPort = lsa->srcPort;
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
    newObj->src = src;
    newObj->srcPort = srcPort;
    newObj->dest = NULL;
    newObj->destPort = 0;
    //Meta
    gettimeofday(&(newObj->timestamp), NULL);
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

void printLSA(LSA *lsa)
{
    printf("==LSA== ");
    printf("src=%s:%d;dest=%s:%d;", lsa->src, lsa->srcPort, lsa->dest, lsa->destPort);
    printf("version=%d;TTL=%d;type=%d;", lsa->version, lsa->TTL, lsa->type);
    printf("senderID=%d;seqNo=%d;", lsa->senderID, lsa->seqNo);
    printf("numLink=%d;numObj=%d;", lsa->numLink, lsa->numObj);
    printf("hasRetran=%d;isDown=%d", lsa->hasRetran, lsa->isDown);

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
    printLSA(lsa);
    packetSize = ptr - buffer;
    if(packetSize > *bufSize) {
        printf("Packet too big, %zu / %zu\n", packetSize, *bufSize);
        *bufSize = -1;
    } else {
        printf("Size: %zu bytes\n", packetSize);
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



