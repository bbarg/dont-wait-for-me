# Makefile for lcrq-server
#
# If you want to set any compile-time flags (like turning on CAS stats
# for the lcrq), simply run
# 	make <target> -DOPT="<options>"

CC=gcc
INCLUDES=-I./synch1.0.1
CFLAGS=-Wall -g $(INCLUDES) $(OPT)
LDLIBS=-pthread
LDFLAGS=-g

OBJ=	\
	http-server \
	lcrq-server \
	msq

.PHONY: clean all debug
all: lcrq-server msq http-server

http-server: libwait.o


lcrq-server: lcrq.o

msq: ms-queue-http-server.o libwait.o
	gcc $(LDFLAGS) ms-queue-http-server.o libwait.o $(LDLIBS) -o msq

debug:
	make OPT="-DDEBUG"

clean:
	rm -rf $(OBJ) *.o
