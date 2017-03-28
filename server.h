#ifndef __SERVER_H__
#define __SERVER_H__

#include "csapp.h"

struct Request{
	char * cmd;
	char * filename;
	char * content;
	int connfd;
	struct stat sbuf;
};
void get();
void readRequest();
void handler(int sig);
void stop(int sig);
void freeRequest(struct Request * r);


#endif