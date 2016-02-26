CC = gcc
SOURCE = allcalls.c
LIBS += -lpopt -lpthread
CFLAGS = -g
BIN = allcalls
BUILDDIR = bin

all: dir $(BUILDDIR)/$(BIN)

dir:
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/$(BIN):
	$(CC) $(CFLAGS) $(SOURCE) -o $(BUILDDIR)/$(BIN) $(LIBS)

clean:
	rm -rf $(BUILDDIR)
