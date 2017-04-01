CC=gcc
CFLAGS=-Wall
LIBS=-lpthread

all: server client


server: server.o csapp.o
		$(CC) $(CFLAGS) -o server server.o csapp.o $(LIBS)

client: client.o csapp.o
		$(CC) $(CFLAGS) -o client  client.o csapp.o $(LIBS)

client.o: client.c csapp.c client.h csapp.h
		$(CC) $(CFLAGS) -c client.c csapp.c $(LIBS)

server.o: server.c csapp.c server.h csapp.h
		$(CC) $(CFLAGS) -c server.c csapp.c $(LIBS)

csapp.o: csapp.c csapp.h
		$(CC) $(CFLAGS) -c csapp.c $(LIBS)

clean:
	rm server client *.o
