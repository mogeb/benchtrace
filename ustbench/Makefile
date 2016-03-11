
all: ustbench

ustbench: ustbench.o tp.o
	gcc -o ustbench ustbench.o tp.o -ldl -llttng-ust

ustbench.o: ustbench.c
	gcc -I. -c -o ustbench.o ustbench.c

tp.o:
	gcc -I. -c -o tp.o tp.c

clean:
	rm *.o
	rm ustbench
