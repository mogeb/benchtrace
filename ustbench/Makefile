CFLAGS=-g -O0

all: ustbench

ustbench: ustbench.o tp.o
	gcc $(CFLAGS) -o ustbench ustbench.o tp.o -ldl -llttng-ust -lpthread -lpopt

ustbench.o: ustbench.c
	gcc $(CFLAGS) -I. -I/usr/include -c -o ustbench.o ustbench.c

ustbench.i: ustbench.c
	gcc -E ustbench.c > ustbench.i

tp.o:
	gcc $(CFLAGS) -I. -c -o tp.o tp.c

clean:
	rm *.o
	rm ustbench
