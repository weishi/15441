#ifndef _SPIFFY_H_
#define _SPIFFY_H_

#include <sys/types.h>
#include <sys/socket.h>
#include "debug.h"

struct spiffy_header_s {
	int ID;
	int lSrcAddr;
	int lDestAddr;
	short lSrcPort;
	short lDestPort;
};
typedef struct spiffy_header_s spiffy_header;

ssize_t spiffy_sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);
int spiffy_recvfrom (int socket, void *buffer, size_t size, int flags, struct sockaddr *addr, socklen_t *lengthptr);
int spiffy_init (long lNodeID, const struct sockaddr *addr, socklen_t addrlen);

#endif /* _SPIFFY_H_ */
