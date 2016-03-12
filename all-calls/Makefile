CC = gcc
SOURCE = allcalls.c
LIBS += -lpopt -lpthread
CFLAGS = -g
BIN = allcalls
BUILDDIR = bin

all: $(BIN)

$(BIN): | $(BUILDDIR)
	$(CC) $(CFLAGS) $(SOURCE) -o $(BUILDDIR)/$(BIN) $(LIBS)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)
