# Makefile
# building lock-free learning programs

CC=gcc
CFLAGS=-Wall -g
LFLAGS=-lpthread

OBJS= \
	learncas	\
	learnqueue

all: $(OBJS)

.PHONY: clean
clean:
	rm -rf $(OBJS)
