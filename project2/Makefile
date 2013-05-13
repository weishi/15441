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
CFLAGS=-Wall -Werror -Wextra -Wshadow -Wunreachable-code -g -D_FORTIFY_SOURCE=2
SOURCE=src
VPATH=$(SOURCE)
OBJECTS = routed.o
OBJECTS += routingEngine.o
OBJECTS += routingTable.o
OBJECTS += resourceTable.o
OBJECTS += connHandler.o
OBJECTS += connObj.o
OBJECTS += flaskParser.o
OBJECTS += linkedList.o
OBJECTS += LSA.o
OBJECTS += OSPF.o


default: routed

routed: $(OBJECTS)
	$(CC) $(CFLAGS) -o routed $(LFLAGS) $(OBJECTS)

$(SOURCE)/%.o: %.c
	$(build-cmd)

clean:
	rm -f routed
	rm ./*.o

