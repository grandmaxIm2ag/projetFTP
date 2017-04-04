#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "csapp.h"

struct Request{
	char * cmd;
	char * filename;
	char * content;
	int clientfd;
	struct stat sbuf;
};

void freeRequest(struct Request* r/* arguments */);
#endif
