#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>

#include "spiffy.h"

#define PACKETLEN 1500
#define BUFLEN 100

typedef struct header_s {
  short magicnum;
  char version;
  char packet_type;
  short header_len;
  short packet_len; 
  u_int seq_num;
  u_int ack_num;
} header_t;  

typedef struct data_packet {
  header_t header;
  char data[BUFLEN];
} data_packet_t;


int main(int argc, char **argv) {
  struct sockaddr_in addr, from;
  socklen_t fromlen;
  char buf[PACKETLEN];
  int sock;
  struct sockaddr_in myaddr;
  fd_set readfds;
  struct user_iobuf *userbuf;
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  data_packet_t *curr;
    
  if (argc < 3) {
    printf("usage: %s <node id> <port>\n", argv[0]);
    return 1;
  }
    
  bzero(&myaddr, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  inet_aton("127.0.0.1", &myaddr.sin_addr);
  myaddr.sin_port = htons(atoi(argv[2]));
  
  if (bind(fd, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
    perror("peer_run could not bind socket");
  }
  
  /* init spiffy */
  spiffy_init(atoi(argv[1]), (struct sockaddr *) &myaddr, sizeof(myaddr));

  while (1) {
    int nfds;
    FD_SET(fd, &readfds);
    
    nfds = select(fd+1, &readfds, NULL, NULL, NULL);
    
    if (nfds > 0) {
	  spiffy_recvfrom(fd, buf, BUFLEN, 0, (struct sockaddr *) &from, &fromlen);	
	  curr = (data_packet_t*)buf;
	  printf("MAGIC: %d\n", ntohs((curr->header).magicnum));
	  fflush(stdout);
    }
	// check timers
  }

  return 0;
}
