include Makefile.inc

BINDIR = bin
BIN = extrae-bench
USTPERF_DIR = ../ustbench
USTPERF_BIN = libustperf.o
USTPERF = $(USTPERF_DIR)/$(USTPERF_BIN)
LIBS = -Wl,-rpath -Wl,$(EXTRAE_HOME)/lib -L$(EXTRAE_HOME)/lib -lpttrace
CFLAGS = -O -I$(EXTRAE_HOME)/include -pthread -I../utils -I../ustbench
USTPERF_CFLAGS = -I../utils

targets: $(BIN)
debug: CFLAGS += -g

$(BIN): extrae-bench.c libustperf.o | $(BINDIR)
	$(CC) $< $(CFLAGS) $(LIBS) $(USTPERF) -o $(BINDIR)/$@  -lpopt

run-extrae-bench:
	EXTRAE_CONFIG_FILE=extrae.xml $(BINDIR)/$(BIN) -n 14999 -p 1

merge-extrae-bench:
	$(EXTRAE_HOME)/bin/mpi2prv set-0/*.mpit -e $(BINDIR)/$(BIN)

libustperf.o: $(USTPERF_DIR)/libustperf.c
	$(CC) $(USTPERF_CFLAGS) -c -o $(USTPERF) $(USTPERF_DIR)/libustperf.c

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -fr set-0 *.prv *.pcf *.row TRACE.mpits TRACE.sym
	rm -rf $(BINDIR)
