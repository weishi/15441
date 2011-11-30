
define build-cmd
$(CC) $(CLFAGS) $< -o $@
endef

CC = gcc
CFLAGS=-Wall -Werror -Wextra -Wshadow -Wunreachable-code -g -D_FORTIFY_SOURCE=2
LDFLAGS	= -lm
SOURCE=src
VPATH=$(SOURCE)

MK_CHUNK_OBJS   = make_chunks.o chunk.o sha.o
OBJS= peer.o
OBJS+=bt_parse.o
OBJS+=spiffy.o
OBJS+=debug.o
OBJS+=input_buffer.o
OBJS+=chunk.o
OBJS+=sha.o

OBJS+=window.o
OBJS+=connPool.o
OBJS+=queue.o
OBJS+=packet.o
OBJS+=congestCtrl.o

BINS = peer make-chunks

$(SOURCE)/%.o: %.c
	$(build-cmd)

default: peer clean-o 

peer: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

clean-o:
	rm -f *.o

make-chunks: $(MK_CHUNK_OBJS)
	$(CC) $(CFLAGS) $(MK_CHUNK_OBJS) -o $@ $(LDFLAGS)

clean:
	rm -f *.o $(BINS) 

debug-text.h: debug.h
	./debugparse.pl < debug.h > debug-text.h


