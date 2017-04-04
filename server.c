#include "server.h"
#include <string.h>
#include <stdio.h>
#include "csapp.h"
#include <stdlib.h>
#define MAX_NAME_LEN 256
#define MAXSEND 256
#define NB_PROC 1



struct Request* req;
pid_t child[NB_PROC];

int main(int argc, char **argv)
{
	Signal(SIGINT, stop);
	Signal(SIGCHLD, handler);
    pid_t p;
    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

		if(argc == 1){
				printf("Usage : %s <port>\n", argv[0]);
				exit(0);
		}

		port = atoi(argv[1]);
    clientlen = (socklen_t)sizeof(clientaddr);

    listenfd = Open_listenfd(port);
    for(int i=0; i<NB_PROC; i++){
        if((p=Fork())==0){
            while (1) {
                if((connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen))>=0){
									//On accept la connexion entrnte

                    /* determine the name of the client */
                    Getnameinfo((SA *) &clientaddr, clientlen,client_hostname, MAX_NAME_LEN, 0, 0, 0);

                    /* determine the textual representation of the client's IP address */
                    Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,INET_ADDRSTRLEN);


                    printf("server connected to %s (%s) %d\n", client_hostname,client_ip_string, getpid());
                    req = malloc(sizeof(struct Request));
                    req->connfd = connfd;

										req->fini=0;

										while(!req->fini){
                    	readRequest(req); //On lit la requete du client
										}
										printf("fini\n");
                }
		    				freeRequest(req);
            }
            exit(0);
        }
        child[i] = p;
    }

    for(int i=0; i<NB_PROC; i++)
        waitpid(child[i], NULL, 0);
		//On attend tous les fils

    exit(0);
}

void handler(int sig){
	//On s'occupe des zombis
    waitpid(-1, NULL, WNOHANG|WUNTRACED);
    return;
}
void stop(int sig){
	//Si on recoit un SIGCTR, on tue tous les fils
    for(int i=0; i<NB_PROC; i++){
        kill(SIGKILL, child[i]);
    }
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
		//On recupere la requete , et remplis notre Request
		stat(req->filename, &req->sbuf);
    if(!strcmp("get", req->cmd)){
			int l = req->sbuf.st_size;
			rio_writen(req->connfd, &l, sizeof(int));
			get(req);
    }else if (!strcmp("bye", req->cmd) ) {
    	printf("fin de la connexion\n");
			Close(req->connfd);
			req->fini=1;
    }else;
    fflush(stdout);
}

void get(struct Request *req){
	char buf[MAXSEND];
	size_t n = 0,err;
	rio_t rio;
	int fd = open(req->filename, O_RDONLY);
	Rio_readinitb(&rio, fd);
	/*
	Tant qu'on a ecris, on continue d'ecrire
	*/
	do {
			n = rio_readn(fd, buf, MAXSEND);
			err = rio_writen(req->connfd, buf, n);
	    if(err == -1){//On stopper si le client s'arrete
					req->fini=1;
		     	fprintf(stderr, "Arret innatendu du client\n");
		     	break;
	    }
	}while (n>0);

	close(fd);

}

void freeRequest(struct Request *r){
	free(r->cmd);
	free(r->filename);
	free(r->content);
	free(r);
}
