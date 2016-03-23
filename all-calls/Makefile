CC = gcc
SOURCE = allcalls.c
LIBS += -lpopt -lpthread
CFLAGS = -g
BIN = allcalls
BUILDDIR = bin

all: $(BIN)

dir:
	mkdir -p $(BUILDDIR)

$(BIN): | $(BUILDDIR)
	$(CC) $(CFLAGS) $(SOURCE) -o $(BUILDDIR)/$(BIN) $(LIBS)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)
