CC = gcc
SOURCE = client.c
LIBS += -lpopt -lpthread
CFLAGS = -g
BIN = singlecall

all:
	$(CC) $(CFLAGS) client.c -o $(BIN) $(LIBS)

clean:
	rm $(BIN)
