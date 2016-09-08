LIBS += -lpthread -lpopt
CFLAGS = -I../utils -I../ustbench
BIN = vampirbench
BINDIR = bin
CC = vtcc
#VTFLAGS = -vt:inst manual -DMANUAL
VTFLAGS = -DMANUAL
USTPERF_DIR = ../ustbench
USTPERF_BIN = libustperf.o
USTPERF = $(USTPERF_DIR)/$(USTPERF_BIN)
USTPERF_CFLAGS = -I../utils

all: $(BIN)

debug: CFLAGS += -g
debug: all

off: | $(BINDIR)
	gcc -g -o $(BINDIR)/$(BIN) vampirbench.c $(LIBS)

$(BIN): vampirtrace.o libustperf.o | $(BINDIR)
	$(CC) $(VTFLAGS) $(CFLAGS) -o $(BINDIR)/$(BIN) vampirbench.o $(USTPERF) -lpopt -DVTRACE
#	$(CC) $(VTFLAGS) vampirbench.c -o $(BINDIR)/$(BIN) -lpopt

vampirtrace.o:
	$(CC) $(VTFLAGS) $(CFLAGS) -c -o vampirbench.o vampirbench.c -DVTRACE

libustperf.o: $(USTPERF_DIR)/libustperf.c
	$(CC) $(USTPERF_CFLAGS) -c -o $(USTPERF) $(USTPERF_DIR)/libustperf.c

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -f *.z
	rm -f *.otf
	rm -f *.thumb
	rm -rf $(BINDIR)
