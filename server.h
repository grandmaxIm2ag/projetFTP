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
	int fini;
	struct stat sbuf;
};
void get(struct Request *req);
void put(struct Request *req);
void readRequest(struct Request *req, int masterReq);
void handler(int sig);
void debloque(int sig);
void stop(int sig);
void freeRequest(struct Request * r);

int auth(char * login);

#endif
