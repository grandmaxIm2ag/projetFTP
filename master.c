#include "server.h"
#include <string.h>
#include <stdio.h>
#include "csapp.h"
#include <stdlib.h>
#define MAX_NAME_LEN 256
#define MAXSEND 256
#define NB_PROC 1



struct Request* req;

int slave[NB_PROC];

int main(int argc, char **argv)
{
	 Signal(SIGINT, stop);
	  Signal(SIGCHLD, handler);
    int listenfd, connfd, portCourant = 0, port = 2121, clientfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    clientlen = (socklen_t)sizeof(clientaddr);

    listenfd = Open_listenfd(port);

    for(int i =0; i<NB_PROC; i++){
        slave[i] = port+i+1;
    }
    while(1){
      if((connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen))>=0){

          /* determine the name of the client */
          Getnameinfo((SA *) &clientaddr, clientlen,client_hostname, MAX_NAME_LEN, 0, 0, 0);

          /* determine the textual representation of the client's IP address */
          Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,INET_ADDRSTRLEN);

          getsockname(connfd, (SA *)&clientaddr,&clientlen);

          printf("server connected to %s (%s) %d\n", client_hostname,client_ip_string, getpid());
          req = malloc(sizeof(struct Request));
          req->connfd = connfd;
          memcpy(req->host, client_hostname, strlen(client_hostname));
          req->port = ntohs(clientaddr.sin_port);

          Close(connfd);

          clientfd = open_clientfd("127.0.0.1", slave[portCourant]);

          rio_t rio;
          Rio_readinitb(&rio, clientfd);

          rio_writen(clientfd, req, sizeof(struct Request));
          portCourant = ++portCourant % NB_PROC;
          Close(clientfd);

          printf("Ici ? \n");
      }
    }

    exit(0);
}

void handler(int sig){
    waitpid(-1, NULL, WNOHANG|WUNTRACED);
    return;
}
void stop(int sig){
    exit(0);
}

void readRequest(struct Request *req){

    size_t n;
    req->cmd = (char*)malloc(sizeof(char)*MAXLINE);
    req->filename = (char*)malloc(sizeof(char)*MAXLINE);
    req->content = (char*)malloc(sizeof(char)*MAXLINE);
    char* request = (char *)malloc(sizeof(char)*MAXLINE);
    rio_t rio;
    Rio_readinitb(&rio, req->connfd);
    n=Rio_readlineb(&rio, request, MAXLINE);
    fflush(stdout);
    request[n-1] = '\0';

    sscanf(request, "%s %s %s", req->cmd, req->filename, req->content);
		stat(req->filename, &req->sbuf);
    fflush(stdout);
}

void freeRequest(struct Request *r){
	free(r->cmd);
	free(r->filename);
	free(r->content);
	free(r);
}
