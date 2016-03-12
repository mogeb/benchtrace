LIBS += -ldl -llttng-ust -lpthread -lpopt
CFLAGS = -g
BIN = ustbench
BINDIR = bin

all: $(BIN)

$(BIN): ustbench.o tp.o | $(BINDIR)
	gcc $(CFLAGS) -o $(BINDIR)/$(BIN) ustbench.o tp.o $(LIBS)

ustbench.o: ustbench.c
	gcc $(CFLAGS) -I. -I/usr/include -c -o ustbench.o ustbench.c

ustbench.i: ustbench.c
	gcc -E ustbench.c > ustbench.i

tp.o:
	gcc $(CFLAGS) -I. -c -o tp.o tp.c

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm *.o
	rm -rf $(BINDIR)
