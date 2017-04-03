CC=gcc
CFLAGS=-Wall
LIBS=-lpthread

all: server client master debugServer rapport

rapport: rapport.tex
		pdflatex rapport.tex
		
debugServer: server.c server.h csapp.c
		$(CC) $(CFLAGS) -g -o debug server.c csapp.c -lpthread

master: master.o csapp.o tell_wait.o
			$(CC) $(CFLAGS) -o master master.o csapp.o tell_wait.o  $(LIBS)

server: server.o csapp.o tell_wait.o
		$(CC) $(CFLAGS) -o server server.o csapp.o tell_wait.o  $(LIBS)

client: client.o csapp.o
		$(CC) $(CFLAGS) -o client  client.o csapp.o $(LIBS)

client.o: client.c csapp.c client.h csapp.h
		$(CC) $(CFLAGS) -c client.c csapp.c $(LIBS)

server.o: server.c csapp.c server.h csapp.h
		$(CC) $(CFLAGS) -c server.c csapp.c $(LIBS)

master.o: master.c csapp.c server.h csapp.h
	  $(CC) $(CFLAGS) -c master.c csapp.c $(LIBS)

tell_wait.o: tell_wait.c csapp.c tell_wait.h csapp.h
	$(CC) $(CFLAGS) -c tell_wait.c csapp.c $(LIBS)

csapp.o: csapp.c csapp.h
		$(CC) $(CFLAGS) -c csapp.c $(LIBS)

clean:
	rm server client *.o *.aux *.log
