LIBS += -ldl -llttng-ust -lpthread -lpopt
CFLAGS = -g
BIN = ustbench
BINDIR = bin

all: $(BIN)

$(BIN): ustbench.o tp.o | $(BINDIR)
	gcc $(CFLAGS) -o $(BINDIR)/$(BIN) ustbench.o tp.o lightweight-ust.o -ldl -llttng-ust -lpthread -lpopt

ustbench.o: ustbench.c lightweight-ust.o
	gcc $(CFLAGS) -I. -I/usr/include -c -o ustbench.o ustbench.c 

ustbench.i: ustbench.c
	gcc -E ustbench.c > ustbench.i

ustbench.s: ustbench.c
	gcc -S ustbench.c

tp.o:
	gcc $(CFLAGS) -I. -c -o tp.o tp.c

lightweight-ust.o: lightweight-ust.c
	gcc $(CFLAGS) -c -o lightweight-ust.o lightweight-ust.c

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm *.o
	rm -rf $(BINDIR)
