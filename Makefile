CC=gcc
CFLAGS=-Wall
LIBS=-lpthread

all: server serveri server2 client


server2: echoserver2.o csapp.o
		$(CC) $(CFLAGS) -o server2 echoserver2.o csapp.o $(LIBS)

server: echoserver.o echo.o csapp.o
		$(CC) $(CFLAGS) -o server echoserver.o echo.o csapp.o $(LIBS)
		
serveri: echoserveri.o csapp.o
		$(CC) $(CFLAGS) -o serveri echoserveri.o csapp.o $(LIBS)

client: echoclient.o echo.o csapp.o
		$(CC) $(CFLAGS) -o client  echoclient.o csapp.o $(LIBS)
		
echoserver2.o: echoserver2.c csapp.c
		$(CC) $(CFLAGS) -c echoserver2.c csapp.c $(LIBS)

echoserver.o: echoserver.c echo.c csapp.c
		$(CC) $(CFLAGS) -c echoserver.c echo.c csapp.c $(LIBS)
	
echoclient.o: echoclient.c  csapp.c
		$(CC) $(CFLAGS) -c echoclient.c csapp.c $(LIBS)
		
echoserveri.o: echoserveri.c csapp.c
		$(CC) $(CFLAGS) -c echoserveri.c csapp.c $(LIBS)

echo.o: echo.c
		$(CC) $(CFLAGS) -c echo.c

csapp.o: csapp.c
		$(CC) $(CFLAGS) -c csapp.c $(LIBS)

clean:
	rm server serveri server2 client *.o
