CC=gcc
CFLAGS=-Wall
LIBS=-lpthread

all: server client


server: server.o csapp.o
		$(CC) $(CFLAGS) -o server server.o csapp.o $(LIBS)

client: client.o csapp.o
		$(CC) $(CFLAGS) -o client  client.o csapp.o $(LIBS)

server.o: server.c csapp.c
		$(CC) $(CFLAGS) -c server.c csapp.c $(LIBS)
	
csapp.o: csapp.c
		$(CC) $(CFLAGS) -c csapp.c $(LIBS)

clean:
	rm server client *.o
