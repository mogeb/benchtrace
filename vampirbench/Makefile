LIBS += -lpthread -lpopt
CFLAGS = -g
BIN = vampirbench
BINDIR = bin
CC = vtcc
#VTFLAGS = -vt:inst manual -DMANUAL
VTFLAGS = -DMANUAL

all: $(BIN)

off: | $(BINDIR)
	gcc -g -o $(BINDIR)/$(BIN) vampirbench.c $(LIBS)

$(BIN): vampirtrace.o | $(BINDIR)
	$(CC) $(VTFLAGS) -g -o $(BINDIR)/$(BIN) vampirbench.o -lpopt -DVTRACE
#	$(CC) $(VTFLAGS) vampirbench.c -o $(BINDIR)/$(BIN) -lpopt

vampirtrace.o:
	$(CC) $(VTFLAGS) -g -c -o vampirbench.o vampirbench.c -DVTRACE

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -f *.z
	rm -f *.otf
	rm -f *.thumb
	rm -rf $(BINDIR)
