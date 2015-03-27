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

.PHONY: clean all
all: lcrq-server http-server

lcrq-server: lcrq.o

msq: ms-queue-http-server.o

clean:
	rm -rf $(OBJ) *.o
