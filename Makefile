#
# Makefile for Project 1
#
# Wei Shi <weishi@andrew.cmu.edu>
#

define build-cmd
$(CC) $(CLFAGS) $< -o $@
endef

CC=gcc
CFLAGS=-Wall -Werror -Wextra -Wshadow -Wunreachable-code -g -D_FORTIFY_SOURCE=2
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



default: lisod

lisod: $(OBJECTS)
	$(CC) $(CFLAGS) -o lisod $(OBJECTS)

$(SOURCE)/%.o: %.c
	$(build-cmd)

clean:
	rm -f lisod
	rm ./*.o

re: clean lisod
