#include "server.h"
#include <string.h>
#include <stdio.h>
#include "csapp.h"
#include <stdlib.h>
#define MAX_NAME_LEN 256
#define MAXSEND 100
#define NB_PROC 10
#define port 2121



struct Request* req;


pid_t child[NB_PROC];

int main(int argc, char **argv)
{
	Signal(SIGINT, stop);
	Signal(SIGCHLD, handler); 
    pid_t p;
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];
    
    clientlen = (socklen_t)sizeof(clientaddr);

    listenfd = Open_listenfd(port);
    for(int i=0; i<NB_PROC; i++){
        if((p=Fork())==0){
            while (1) {
                if((connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen))>=0){
                	
                    /* determine the name of the client */
                    Getnameinfo((SA *) &clientaddr, clientlen,client_hostname, MAX_NAME_LEN, 0, 0, 0);
                    
                    /* determine the textual representation of the client's IP address */
                    Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,INET_ADDRSTRLEN);
                    
                    
                    printf("server connected to %s (%s) %d\n", client_hostname,client_ip_string, getpid());
                    req = malloc(sizeof(struct Request));
                    req->connfd = connfd;

                    readRequest(connfd);
                    Close(connfd);
                }
            }
    		freeRequest(req);
            exit(0);
        }
        child[i] = p;
    }
    
    for(int i=0; i<NB_PROC; i++)
        waitpid(child[i], NULL, 0);
    
    exit(0);
}

void handler(int sig){
    waitpid(-1, NULL, WNOHANG|WUNTRACED);
    return;
}
void stop(int sig){
    for(int i=0; i<NB_PROC; i++){
        kill(SIGKILL, child[i]);
    }
    exit(0);
}

void readRequest(){

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
	
	printf("passé : %s\n",req->cmd );
    stat(req->filename, &req->sbuf);
    if(!strcmp("get", req->cmd)){
    	get();
    }else{
    	printf("La commande %s non defini", req->cmd);
    }
    fflush(stdout);
}

void get(){
	char buf[MAXSEND];
	size_t n,err;
	rio_t rio;
	
	/*
	On envoie d'un coup
	int srcfd;
    char *srcp;

    srcfd = Open(req->filename, O_RDONLY, 0);
    srcp = Mmap(0, req->sbuf.st_size, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_writen(req->connfd, srcp, req->sbuf.st_size);
    Munmap(srcp, req->sbuf.st_size);
    */

    int fd = open(req->filename, O_RDONLY);
    Rio_readinitb(&rio, fd);

    while ((n = Rio_readlineb(&rio, buf, MAXSEND)) > 0) {
        err = io_writen(req->connfd, buf, n);
        printf("%s",buf );
        if(err == 1){
        	fprintf(stderr, "Arret innatendu du client\n");
        	break;
        }
    }
    
}

void freeRequest(struct Request *r){
	free(r->cmd);
	free(r->filename);
	free(r->content);
	free(r);
}