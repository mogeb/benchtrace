LIBS += -ldl -llttng-ust -lpthread -lpopt
CFLAGS = -I../utils
BIN = ustbench
BINDIR = bin
LIBS = -ldl -llttng-ust -lpthread -lpopt -lurcu-bp

all: $(BIN)

debug: CFLAGS += -g
debug: all

$(BIN): ustbench.o tp.o | $(BINDIR)
	gcc $(CFLAGS) -o $(BINDIR)/$(BIN) ustbench.o tp.o lightweight-ust.o libustperf.o $(LIBS)

ustbench.o: ustbench.c lightweight-ust.o libustperf.o
	gcc $(CFLAGS) -I. -I/usr/include -c -o ustbench.o ustbench.c 

ustbench.i: ustbench.c
	gcc -E ustbench.c > ustbench.i

ustbench.s: ustbench.c
	gcc -S ustbench.c

tp.o:
	gcc $(CFLAGS) -I. -c -o tp.o tp.c

lightweight-ust.o: lightweight-ust.c
	gcc $(CFLAGS) -c -o lightweight-ust.o lightweight-ust.c

libustperf.o: libustperf.c
	gcc $(CFLAGS) -c -o libustperf.o libustperf.c

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm *.o
	rm -rf $(BINDIR)
