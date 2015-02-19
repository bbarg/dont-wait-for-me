# Makefile
# building lock-free learning programs

CC=gcc

INCLUDES = 

CFLAGS=-Wall -g $(INCLUDES)
LDFLAGS=-g 
LDLIBS=-lpthread

main: test_queue learncas

test_queue: test_queue.o learnqueue.o 

test_queue.o: test_queue.c learnqueue.h

learnqueue.o: learnqueue.c learnqueue.h

learncas: 

.PHONY: clean
clean:
	rm -rf *.o test_queue learnqueue learncas core a.out 
.PHONY: all 
all: clean main 
