#ifndef __SERVER_H__
#define __SERVER_H__

#include "csapp.h"

struct Request{
	char * cmd;
	char * filename;
	char * content;
	char * host;
	int connfd;
	int port;
	struct stat sbuf;
};
void get(struct Request *req);
void readRequest(struct Request *req);
void handler(int sig);
void debloque(int sig);
void stop(int sig);
void freeRequest(struct Request * r);


#endif
