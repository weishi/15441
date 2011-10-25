#
# Makefile for Project 2
# CCN
#
# Wei Shi <weishi@andrew.cmu.edu>
# Han Liu <hanl1@andrew.cmu.edu>

define build-cmd
$(CC) $(CLFAGS) $< -o $@
endef

CC=gcc
CFLAGS=-Wall -Werror -Wextra -Wshadow -Wunreachable-code -O2 -D_FORTIFY_SOURCE=2
SOURCE=src
VPATH=$(SOURCE)
OBJECTS = routed.o
OBJECTS += linkedList.o
OBJECTS += fileIO.o
OBJECTS += common.o
OBJECTS += daemonize.o


default: routed

lisod: $(OBJECTS)
	$(CC) $(CFLAGS) -o ../routed $(LFLAGS) $(OBJECTS)

$(SOURCE)/%.o: %.c
	$(build-cmd)

clean:
	rm ./*.o

