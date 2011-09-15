################################################################################
# Makefile                                                                     #
#                                                                              #
# Description: This file contains the make rules for Recitation 1.             #
#                                                                              #
# Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                          #
#          Wolf Richter <wolf@cs.cmu.edu>                                      #
#                                                                              #
################################################################################

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
OBJECTS += httpHandler.o
OBJECTS += connObj.o

default: lisod

lisod: $(OBJECTS)
	$(CC) $(CFLAGS) -o lisod $(OBJECTS)

$(SOURCE)/%.o: %.c
	$(build-cmd)

clean:
	rm -f lisod
	rm ./*.o
