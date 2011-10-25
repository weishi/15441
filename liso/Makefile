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
OBJECTS = lisod.o
OBJECTS += selectEngine.o
OBJECTS += linkedList.o
OBJECTS += connHandler.o
OBJECTS += connObj.o
OBJECTS += httpParser.o
OBJECTS += httpHeader.o
OBJECTS += httpResponder.o
OBJECTS += fileIO.o
OBJECTS += common.o
OBJECTS += sslLib.o
OBJECTS += daemonize.o

LFLAGS=-lssl

default: lisod

lisod: $(OBJECTS)
	$(CC) $(CFLAGS) -o ../lisod $(LFLAGS) $(OBJECTS)

$(SOURCE)/%.o: %.c
	$(build-cmd)

clean:
	rm -f lisod
	rm ./*.o

