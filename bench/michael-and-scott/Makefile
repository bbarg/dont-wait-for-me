LDFLAGS =  -lmpc -lm

all:
	as -c -o cas.o cas.s -mips2
	as -c -o tas.o tas.s -mips2
	as -c -o fai.o fai.s -mips2
	as -c -o cas.o cas.s -mips2
	cc -O2 main.c args.c my_queue.c backoff.c cas.o tas.o fai.o -o queue $(LDFLAGS)

clean:
	rm -f *.o *~ core
