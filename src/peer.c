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
  int i;
  fillChunkList(&masterChunk, MASTER, config->chunk_file);
  fillChunkList(&hasChunk, HAS, config->has_chunk_file);
  fillPeerList(config);

  if((log_file = fopen("window_monitor.log", "a")) == NULL){
    fprintf(stderr, "Failed to open logging file."
            "All messages will redirect to stderr\n");
  };


  maxConn = config->max_conn;
  nonCongestQueue = newqueue();
  for(i = 0; i < peerInfo.numPeer; i++) {
    int peerID = peerInfo.peerList[i].peerID;
    uploadPool[peerID].dataQueue = newqueue();
    uploadPool[peerID].ackWaitQueue = newqueue();
    uploadPool[peerID].connected = 0;
    uploadPool[peerID].connID = 0;
    downloadPool[peerID].getQueue = newqueue();
    downloadPool[peerID].timeoutQueue = newqueue();
    downloadPool[peerID].ackSendQueue = newqueue();
    downloadPool[peerID].connected = 0;
    downloadPool[peerID].cache = NULL;
    initWindows(&(downloadPool[peerID].rw), &(uploadPool[peerID].sw));
  }
  printInit();
}

void printInit()
{
  printChunk(&masterChunk);
  printChunk(&hasChunk);
}
void printChunk(chunkList *list)
{
  int i;
  printf("ListType: %d\n", list->type);
  for(i = 0; i < list->numChunk; i++) {
    char buf[50];
    bzero(buf, 50);
    chunkLine *line = &(list->list[i]);
    binary2hex(line->hash, 20, buf);
    printf("%d: %s\n", line->seq, buf);
  }
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
      char *newline = strrchr(lineBuf, '\n');
      if(newline) {
	*newline = '\0';
      }
      linePtr = &(lineBuf[6]);
      if((masterFile = fopen(linePtr, "r")) == NULL) {
	printf("Error open master data file: <%s>\n", linePtr);
	exit(1);
      }
      list->filePtr = masterFile;
    }
    //Skip "Chunks:" line
    fgets(lineBuf, MAX_LINE_SIZE, fPtr);
  case GET:
    list->getChunkFile=calloc(strlen(filename)+1,1);
    strcpy(list->getChunkFile, filename);
  case HAS:
    if(fPtr == NULL) {
      if((fPtr = fopen(filename, "r")) == NULL) {
	fprintf(stderr, "Open file %s failed\n", filename);
	exit(-1);
      } else {
	printf("Opened %s\n", filename);
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
      hex2binary(hashBuf, 2 * SHA1_HASH_SIZE, cPtr->hash);
      free(hashBuf);
      numChunk++;
    }
    list->numChunk = numChunk;
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
  peerList_t *peerList = peerInfo.peerList;
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
  peerInfo.numPeer = numPeer;
}

void handlePacket(Packet *pkt)
{
  if(verifyPacket(pkt)) {
    int type = getPacketType(pkt);
    switch(type) {
    case 0: { //WHOHAS
      printf("->WHOHAS\n");
      Packet *pktIHAVE = newPacketIHAVE(pkt);
      enqueue(nonCongestQueue, (void *)pktIHAVE);
      break;
    }
    case 1: { //IHAVE
      printf("->IHAVE\n");
      int peerIndex = searchPeer(&(pkt->src));
      int peerID = peerInfo.peerList[peerIndex].peerID;
      printf("Trying to get a chunk from %d\n", peerID);
      newPacketGET(pkt, downloadPool[peerID].getQueue);
      idle = 0;
      break;
    }
    case 2: { //GET
      printf("->GET\n");
      int peerIndex = searchPeer(&(pkt->src));
      int peerID = peerInfo.peerList[peerIndex].peerID;
      //in case the sender has not received the final ack from the receiver but 
      // the receiver is sending out a new get, assume the receiver has received
      // the last packet so abort the wait queue and re-initialize
      clearQueue(uploadPool[peerID].ackWaitQueue); 
      initWindows(&(downloadPool[peerID].rw), &(uploadPool[peerID].sw));
      newPacketDATA(pkt, uploadPool[peerID].dataQueue);
      //set start time
      uploadPool[peerID].connID++;
      gettimeofday(&(uploadPool[peerID].startTime), NULL);
      printf("new data added to Q at peer %d\n", peerID);
      break;
    }
    case 3: { //DATA
      printf("->DATA");
      int peerIndex = searchPeer(&(pkt->src));
      int peerID = peerInfo.peerList[peerIndex].peerID;
      if(1 == updateGetSingleChunk(pkt, peerID)) {
	updateGetChunk();
      }
      break;
    }
    case 4: { //ACK
      printf("->ACK\n");
      int peerIndex = searchPeer(&(pkt->src));
      int peerID = peerInfo.peerList[peerIndex].peerID;
      updateACKQueue(pkt, peerID);
      break;
    }
    case 5://DENIED not used
    default:
      printf("Type=WTF\n");
    }
  } else {
    printf("Invalid packet\n");
  }
  freePacket(pkt);
  return;
}

void updateACKQueue(Packet *pkt, int peerID)
{
  sendWindow *sw = &(uploadPool[peerID].sw);
  uint32_t ack = getPacketAck(pkt);
  queue *ackWaitQueue = uploadPool[peerID].ackWaitQueue;
  queue *dataQueue = uploadPool[peerID].dataQueue;
  Packet *ackWait = peek(ackWaitQueue);
  struct timeval cur_time;
  gettimeofday(&cur_time, NULL);
  logger(peerID, uploadPool[peerID].connID, diffTimevalMilli(&cur_time, &(uploadPool[peerID].startTime)), sw->ctrl.windowSize);
  expandWindow(&(sw->ctrl));
  uploadPool[peerID].timeoutCount = 0;
  printf("Received ACK %d. Last acked %d. Next in ackWaitQueue: %d\n", ack, sw->lastPacketAcked, ackWait == NULL ? 65535 : getPacketSeq(ackWait));
  if(ackWait != NULL){
    if(ack >= getPacketSeq(ackWait)){
      while(ackWait != NULL && ack >= getPacketSeq(ackWait)){
	dequeue(ackWaitQueue);
	freePacket(ackWait);
	ackWait = peek(ackWaitQueue);
      }
      sw->lastPacketAcked = ack;
      updateSendWindow(sw);
      //This is a hack but could be fine 
      //Sender resets sending window if the whole chunk has been sent
      if(ack == BT_CHUNK_SIZE / PACKET_DATA_SIZE + 1){
	printf("Sender finished sending current chunk");
	initWindows(&(downloadPool[peerID].rw), &(uploadPool[peerID].sw));
	assert(dequeue(ackWaitQueue) == NULL);
      }
    } else {//unexpected ACK ack number 
      if(ack == sw->lastPacketAcked) { //dupliate ACK
	sw->dupCount++;
	printf("Received duplicate packets %d\n", ack);
	if(sw->dupCount == MAX_DUPLICATE){ //trigger fast retransmit
	  printf("Received 3 duplicates acks %d retransmitting\n", ack);
	  mergeAtFront(ackWaitQueue, dataQueue);
	  shrinkWindow(&(sw->ctrl));
	}
      }
      else{
	printf("Weird ack %d. last acked %d\n", ack, sw->lastPacketAcked);
      }

    }
  }
  else { //unexpected ACK arrival
    printf("WTF why is there still ACK coming in?\n");
      }
}

int updateGetSingleChunk(Packet *pkt, int peerID)
{
  recvWindow *rw = &(downloadPool[peerID].rw);
  int dataSize = getPacketSize(pkt) - 16;
  uint8_t *dataPtr = pkt->payload + 16;
  uint32_t seq = (uint32_t)getPacketSeq(pkt);
  downloadPool[peerID].timeoutCount = 0;
  printf("Got pkt %d expecting %d\n", seq, rw->nextPacketExpected);
  if(seq >= rw->nextPacketExpected){
    if((seq > rw->nextPacketExpected) && ((seq - rw->nextPacketExpected) <= 64)){//TODO: change this!
      insertInOrder(&(downloadPool[peerID].cache), newFreePacketACK(seq), seq);
      //ASSERSION: under all cases the queue should be empty when this happens
      newPacketACK(rw->nextPacketExpected - 1, downloadPool[peerID].ackSendQueue);
    } else if(seq - rw->nextPacketExpected <= 64){//TODO: change this!
      newPacketACK(seq, downloadPool[peerID].ackSendQueue);
      rw->nextPacketExpected = 
	flushCache(rw->nextPacketExpected, downloadPool[peerID].ackSendQueue, &(downloadPool[peerID].cache));
    }
    rw->lastPacketRead = seq;
    rw->lastPacketRcvd = seq;
    
    int curChunk = downloadPool[peerID].curChunkID;
    long offset = (seq - 1) * PACKET_DATA_SIZE + BT_CHUNK_SIZE * curChunk;
    FILE *of = getChunk.filePtr;
    printf("DataIn %d [%ld-%ld]\n", seq, offset, offset + dataSize);
    if(of != NULL){
      fseek(of, offset, SEEK_SET);
      fwrite(dataPtr, sizeof(uint8_t), dataSize, of);
    }
    
    /*Check if this GET finished */
    //TODO: This is a hack, should be change in CP2
    if((rw->nextPacketExpected > BT_CHUNK_SIZE / PACKET_DATA_SIZE + 1)
       && (downloadPool[peerID].ackSendQueue->size == 0)){
      printf("Asserting chunk finished downloading: next expected %d thresh %d\n", rw->nextPacketExpected, BT_CHUNK_SIZE / PACKET_DATA_SIZE + 1);
      getChunk.list[curChunk].fetchState = 1;
      downloadPool[peerID].state = 0;
      /*Packet *clearPkt = dequeue(downloadPool[peerID].timeoutQueue);
      while(clearPkt != NULL) {
	freePacket(clearPkt);
	clearPkt = dequeue(downloadPool[peerID].timeoutQueue);
	}*/
      clearQueue(downloadPool[peerID].timeoutQueue);
      //clearQueue(downloadPool[peerID].ackSendQueue);
      initWindows(&(downloadPool[peerID].rw), &(uploadPool[peerID].sw));
      printf("Chunk %d fetched\n", curChunk);
      printf("%d More GETs in queue\n", downloadPool[peerID].getQueue->size);
      return 1;//this GET is done
    } else {
      return 0;
    }
  }
  else {//packet seq smaller than expected. Just send back ack.
    printf("Received unexpected packet."
	   "Expecting %d received %d !",
	   rw->nextPacketExpected, seq);
    newPacketACK(seq, downloadPool[peerID].ackSendQueue);
    return 0;
  }
}
/* Check if all GETs are done */
void updateGetChunk()
{
  fflush(log_file);
  int i = 0;
  int done = 1;
  for(i = 0; i < getChunk.numChunk; i++) {
    if(getChunk.list[i].fetchState != 1) {
      done = 0;
      printf("Still missing chunk %d\n", i);
    }
  }
  if(done) {
    fclose(getChunk.filePtr);
    getChunk.filePtr = NULL;
    printf("GOT %s\n", getChunk.getChunkFile);
    free(getChunk.getChunkFile);
    bzero(&getChunk, sizeof(getChunk));
    idle = 1;
  }
}

int searchPeer(struct sockaddr_in *src)
{
  int i = 0;
  for(i = 0; i < peerInfo.numPeer; i++) {
    struct sockaddr_in *entry = &(peerInfo.peerList[i].addr);
    //Compare sin_port & sin_addr.s_addr
    //TODO:
    //somehow packt redirectd from spiffy router does not have sin_addr so
    //im ignoring this field when comparing peers for now. Fix this later
    int isSame = entry->sin_port == src->sin_port;
      //&& entry->sin_addr.s_addr == src->sin_addr.s_addr;
    if(isSame) {
      return i;
    }
  }
  return -1;
}

void process_inbound_udp(int sock)
{
#define BUFLEN 1500
  struct sockaddr_in from;
  socklen_t fromlen;
  char buf[BUFLEN];

  fromlen = sizeof(from);
  spiffy_recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &from, &fromlen);

  /*
    printf("Incoming message from %s:%d\n",
    inet_ntoa(from.sin_addr),
    ntohs(from.sin_port));
  */
  Packet *newPkt = newPacketFromBuffer(buf);
  memcpy(&(newPkt->src), &from, fromlen);
  handlePacket(newPkt);

}


void process_get(char *chunkfile, char *outputfile)
{
  printf("Handle GET (%s, %s)\n", chunkfile, outputfile);

  fillChunkList(&getChunk, GET, chunkfile);
  if((getChunk.filePtr = fopen(outputfile, "w")) == NULL) {
    fprintf(stderr, "Open file %s failed\n", outputfile);
    exit(-1);
  }
  //TODO:only get chunks that I don't have
  printChunk(&getChunk);
  newPacketWHOHAS(nonCongestQueue);

}

void handle_user_input(char *line, void *cbdata)
{
  printf("Handling user input\n");
  char chunkf[128], outf[128];
  cbdata = cbdata;
  bzero(chunkf, sizeof(chunkf));
  bzero(outf, sizeof(outf));

  if (sscanf(line, "GET %120s %120s", chunkf, outf)) {
    if (strlen(outf) > 0) {
      process_get(chunkf, outf);
      //idle = 0;
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
  struct timeval timeout;
  while (1) {
    int nfds;
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(sock, &readfds);

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    nfds = select(sock + 1, &readfds, NULL, NULL, &timeout);
       
    if (nfds > 0) {
      if (FD_ISSET(sock, &readfds)) {
	process_inbound_udp(sock);
      }
      if(FD_ISSET(STDIN_FILENO, &readfds)){
	  printf("detected user input \n");
	}
      if (FD_ISSET(STDIN_FILENO, &readfds) && idle == 1) {
	process_user_input(STDIN_FILENO,
			   userbuf,
			   handle_user_input,
			   "Currently unused");
      }
    }

    flushQueue(sock, nonCongestQueue);
    flushUpload(sock);
    flushDownload(sock);
  }
}

void flushUpload(int sock)
{
  int i = 0;
  Packet *pkt;
  connUp *pool = uploadPool;
  for(i = 0; i < peerInfo.numPeer; i++) {
    int peerID = peerInfo.peerList[i].peerID;
    //Pacekt lies within the sending window
    Packet *ack = peek(pool[peerID].ackWaitQueue);
    if(ack != NULL){
      struct timeval curTime;
      gettimeofday(&curTime, NULL);
      long dt = diffTimeval(&curTime, &(ack->timestamp));
      if(dt > DATA_TIMEOUT_SEC) {//detected timeout
	pool[peerID].timeoutCount++;
	printf("data timeout. waiting for ack %d\n", getPacketSeq(ack));
	if(pool[peerID].timeoutCount == 3){
	  printf("Receiver ID %d timed out 3 times. Closing connection\n", peerID);
	  cleanUpConnUp(&(pool[peerID]));
	  continue;
	}
	shrinkWindow(&(pool[peerID].sw.ctrl));
	mergeAtFront(pool[peerID].ackWaitQueue, pool[peerID].dataQueue);
      }
    }
    pkt = peek(pool[peerID].dataQueue);
    while(pkt != NULL && 
	  (getPacketSeq(pkt) <= pool[peerID].sw.lastPacketAvailable)) { 
      peerList_t *p = &(peerInfo.peerList[i]);
      int retVal = spiffy_sendto(sock,
				 pkt->payload,
				 getPacketSize(pkt),
				 0,
				 (struct sockaddr *) & (p->addr),
				 sizeof(p->addr));
      setPacketTime(pkt);
      printf("Sent data %d. last available %d\n", getPacketSeq(pkt), pool[peerID].sw.lastPacketAvailable);
      if(retVal == -1) { //DATA lost
	break;
      }
      dequeue(pool[peerID].dataQueue);
      pool[peerID].sw.lastPacketSent = getPacketSeq(pkt);
      enqueue(pool[peerID].ackWaitQueue, pkt);
      pkt = peek(pool[peerID].dataQueue);
      printf("data queue size %d\n", pool[peerID].dataQueue->size);
    }
  }
}

void flushDownload(int sock)
{
  int i = 0;
  int idx;
  uint8_t* hash;
  Packet *pkt;
  connDown *pool = downloadPool;
  for(i = 0; i < peerInfo.numPeer; i++) {
    int peerID = peerInfo.peerList[i].peerID;
    /* Send ACK */
    Packet *ack = peek(pool[peerID].ackSendQueue);
    //Maybe should use 'if' instead of 'while' here
    /*ASSUMPTION: 
      does not need to check window boundary here
      because it is checked on the DATA sending end 
      and the rate at which ACKs get sent should always
      be slower than the DATA being sent
    */
    while(ack != NULL) {
      peerList_t *p = &(peerInfo.peerList[i]);
      printf("Sending ack %d\n", getPacketAck(ack));
      int retVal = spiffy_sendto(sock,
				 ack->payload,
				 getPacketSize(ack),
				 0,
				 (struct sockaddr *) & (p->addr),
				 sizeof(p->addr));
      printf("Sent ack %d\n", getPacketAck(ack));
      if(retVal == -1) {
	printf("Sending ACK failed\n");
      } else{
	dequeue(pool[peerID].ackSendQueue);
	freePacket(ack);
	ack = dequeue(pool[peerID].ackSendQueue);
      }
    }
    /* Send GET */
    switch(pool[peerID].state) {
    case 0://Ready for next
      pkt = dequeue(pool[peerID].getQueue);
      while(pkt != NULL){
	hash = getPacketHash(pkt, 0);
	printHash(hash);
	idx = searchHash(hash, &getChunk, 0);
	printf("Search returned %d\n", idx);
	if(idx == -1){ //someone else is sending or has sent this chunk
	  freePacket(pkt); 
	  pkt = dequeue(pool[peerID].getQueue);
	}
	else{
	  getChunk.list[idx].fetchState = 2;
	  break;
	}
      }
      
      if(pkt != NULL) {
	printf("Sending a GET\n");
	peerList_t *p = &(peerInfo.peerList[i]);
	hash = pkt->payload + 16;
	char buf[50];
	bzero(buf, 50);
	binary2hex(hash, 20, buf);
	printf("GET hash:%s\n", buf);
	pool[peerID].curChunkID = searchHash(hash, &getChunk, -1);
	//Send get
	int retVal = spiffy_sendto(sock,
				   pkt->payload,
				   getPacketSize(pkt),
				   0,
				   (struct sockaddr *) & (p->addr),
				   sizeof(p->addr));
	
	if(retVal == -1) {
	  //TODO: this might not be the best solution
	  newPacketWHOHAS(nonCongestQueue);
	  freePacket(pkt);
	  cleanUpConnDown(&(pool[peerID]));
	  return;
	}
	//getChunk.list[pool[peerID].curChunkID].fetchState = 2;
	//Mark time
	setPacketTime(pkt);
	//Put it in timeoutQueue
	enqueue(pool[peerID].timeoutQueue, pkt);
	pool[peerID].state = 1;
      }
      break;
    case 1: {//Waiting
      pkt = peek(pool[peerID].timeoutQueue);
      struct timeval curTime;
      gettimeofday(&curTime, NULL);
      long dt = diffTimeval(&curTime, &(pkt->timestamp));
      if(dt > GET_TIMEOUT_SEC) {
	pool[peerID].timeoutCount++;
	printf("Get requset timed out %d times\n", pool[peerID].timeoutCount);
	setPacketTime(pkt);
	if(pool[peerID].timeoutCount == 3){
	  getChunk.list[pool[peerID].curChunkID].fetchState = 0;
	  pool[peerID].state = 0;
	  newPacketWHOHAS(nonCongestQueue);
	  freePacket(pkt);
	  //mergeAtFront(pool[peerID].timeoutQueue, pool[peerID].getQueue);
	  cleanUpConnDown(&(pool[peerID]));
	}
      }
      break;
      }
      case 2: {
	break;
      }
      default:
	break;
    }

  }
}

void flushQueue(int sock, queue *sendQueue)
{
  int i = 0;
  int retVal = -1;
  int noLoss = 1; //for debug use only. get rid of it later
  int count = sendQueue->size;
  Packet *pkt = dequeue(sendQueue);
  if(pkt == NULL) {
    return;
  }
  peerList_t *list = peerInfo.peerList;
  while(count > 0) {
    printf("Sending %d Packet\n", getPacketType(pkt));
    if(pkt->dest != NULL){ //IHAVE packets have specific destinations
      retVal = spiffy_sendto(sock,
			     pkt->payload,
			     getPacketSize(pkt),
			     0,
			     (struct sockaddr *) (pkt->dest),
			     sizeof(*(pkt->dest)));
    }
    else {
      for(i = 0; i < peerInfo.numPeer; i++) {
	if(list[i].isMe == 0) {
	  retVal = spiffy_sendto(sock,
				 pkt->payload,
				 getPacketSize(pkt),
				 0,
				 (struct sockaddr *) & (list[i].addr),
				 sizeof(list[i].addr));
	  if(retVal == -1) {
	    break;
	  }
	}
      }
    }				
    if(retVal == -1) {
      noLoss = 0;
      if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
	printf("Error EAGAIN\n");
	enqueue(sendQueue, (void *)pkt);
      } else {
	printf("Fail sending\n");
	close(sock);
      }
    }
    freePacket(pkt);
    pkt = dequeue(sendQueue);
    count--;
  }
		
  if(noLoss){
    printf("All packets flushed from nonCongestQueue\n");
  }
}



long diffTimeval(struct timeval *t1, struct timeval *t2)
{
  return t1->tv_sec - t2->tv_sec;
}

int diffTimevalMilli(struct timeval *t1, struct timeval *t2){
  return (t1->tv_sec - t2->tv_sec)*1000 + (t1->tv_usec - t2->tv_usec)/1000.0;
}


