/*
 * Han Liu <hanl1@andrew.cmu.edu>
 * Wei Shi <weishi@andrew.cmu.edu>
 */

#include "peer.h"

void peer_run(bt_config_t *config);

int main(int argc, char **argv)
{

    bt_config_t config;

    bt_init(&config, argc, argv);

    DPRINTF(DEBUG_INIT, "peer.c main beginning\n");

    config.identity = 1; // your group number here
    strcpy(config.chunk_file, "chunkfile");
    strcpy(config.has_chunk_file, "haschunks");

    bt_parse_command_line(&config);

    bt_dump_config(&config);

    init(&config);
    peer_run(&config);
    return 0;
}

void init(bt_config_t *config)
{

    fillChunkList(&masterChunk, 1, NULL);
    fillChunkList(&hasChunk, 2, NULL);

    fillPeerList(config);

    maxConn = config->max_conn;

}

void fillChunkList(chunkList *list, enum chunkType type, char *filename)
{
    FILE *fPtr = NULL;
    char lineBuf[MAX_LINE_SIZE];
    char *linePtr;
    int numChunk = 0;
    int chunkIdx = 0;
    bzero(list, sizeof(list));
    list->type = type;

    switch (type) {
    case MASTER:
        if((fPtr = fopen(filename, "r")) == NULL) {
            printf("Open file %s failed\n", filename);
            exit(1);
        }
        fgets(lineBuf, MAX_LINE_SIZE, fPtr);
        if(strncmp(lineBuf, "File: ", 6) != 0) {
            printf("Error parsing masterchunks\n");
            exit(1);
        } else {
            FILE *masterFile;
            linePtr = &(lineBuf[6]);
            if((masterFile = fopen(linePtr, "r")) == NULL) {
                printf("Error open master data file\n");
                exit(1);
            }
            list->filePtr = masterFile;
        }
        //Skip "Chunks:" line
        fgets(lineBuf, MAX_LINE_SIZE, fPtr);
    case GET:
    case HAS:
        if(fPtr == NULL) {
            if((fPtr = fopen(filename, "r")) == NULL) {
                fprintf(stderr, "Open file %s failed\n", filename);
                exit(-1);
            }
        }
        while(!feof(fPtr)) {
            char *hashBuf;
            if(fgets(lineBuf, MAX_LINE_SIZE, fPtr) == NULL) {
                break;
            }
            if(2 != sscanf(lineBuf, "%d %ms", &chunkIdx, &hashBuf)) {
                printf("Error parsing hash\n");
                exit(1);
            }
            chunkLine *cPtr = &(list->list[numChunk]);
            cPtr->seq = chunkIdx;
            hex2binary(hashBuf, SHA1_HASH_SIZE, cPtr->hash);
            free(hashBuf);
            numChunk++;
        }
        break;
    default:
        printf("WTF\n");
        exit(1);
    }
    fclose(fPtr);
}


void fillPeerList(bt_config_t *config)
{
    bt_peer_t *peer = config->peers;
    int numPeer = 0;
    while(peer != NULL) {
        if(peer->id == config->identity) {
            peerList[numPeer].isMe = 1;
        } else {
            peerList[numPeer].isMe = 0;

        }
        peerList[numPeer].peerID = peer->id;
        memcpy(&(peerList[numPeer].addr), &(peer->addr), sizeof(struct sockaddr_in));
        peer = peer->next;
        numPeer++;
    }
}


void process_inbound_udp(int sock)
{
#define BUFLEN 1500
    struct sockaddr_in from;
    socklen_t fromlen;
    char buf[BUFLEN];

    fromlen = sizeof(from);
    spiffy_recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &from, &fromlen);

    printf("PROCESS_INBOUND_UDP SKELETON -- replace!\n"
           "Incoming message from %s:%d\n%s\n\n",
           inet_ntoa(from.sin_addr),
           ntohs(from.sin_port),
           buf);
}

void process_get(char *chunkfile, char *outputfile)
{
    printf("PROCESS GET SKELETON CODE CALLED.  Fill me in!  (%s, %s)\n",
           chunkfile, outputfile);
}

void handle_user_input(char *line, void *cbdata)
{
    char chunkf[128], outf[128];
    cbdata=cbdata;
    bzero(chunkf, sizeof(chunkf));
    bzero(outf, sizeof(outf));

    if (sscanf(line, "GET %120s %120s", chunkf, outf)) {
        if (strlen(outf) > 0) {
            process_get(chunkf, outf);
        }
    }
}


void peer_run(bt_config_t *config)
{
    int sock;
    struct sockaddr_in myaddr;
    fd_set readfds;
    struct user_iobuf *userbuf;

    if ((userbuf = create_userbuf()) == NULL) {
        perror("peer_run could not allocate userbuf");
        exit(-1);
    }

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) == -1) {
        perror("peer_run could not create socket");
        exit(-1);
    }

    bzero(&myaddr, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(config->myport);

    if (bind(sock, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
        perror("peer_run could not bind socket");
        exit(-1);
    }

    spiffy_init(config->identity, (struct sockaddr *)&myaddr, sizeof(myaddr));

    while (1) {
        int nfds;
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sock, &readfds);

        nfds = select(sock + 1, &readfds, NULL, NULL, NULL);

        if (nfds > 0) {
            if (FD_ISSET(sock, &readfds)) {
                process_inbound_udp(sock);
            }

            if (FD_ISSET(STDIN_FILENO, &readfds)) {
                process_user_input(STDIN_FILENO,
                                   userbuf,
                                   handle_user_input,
                                   "Currently unused");
            }
        }
    }
}
