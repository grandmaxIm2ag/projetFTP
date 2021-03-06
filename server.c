#include "server.h"
#include <string.h>
#include <stdio.h>
#include "csapp.h"
#include <stdlib.h>
#define MAX_NAME_LEN 256
#define MAXSEND 256
#define NB_PROC 1
#define PORT_MASTER 2121



struct Request* req;
char** modifServ;
int chg=0;
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
									//On accept la connexion entrente

                    /* determine the name of the client */
                    Getnameinfo((SA *) &clientaddr, clientlen,client_hostname, MAX_NAME_LEN, 0, 0, 0);

                    /* determine the textual representation of the client's IP address */
                    Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,INET_ADDRSTRLEN);


                    printf("server connected to %s (%s) %d\n", client_hostname,client_ip_string, getpid());
                    req = malloc(sizeof(struct Request));
                    req->connfd = connfd;

										req->fini=0;
										if(!strcmp("127.0.0.1", client_hostname)){//Si c'est le master qui se connecte, on fait tous les changements
												rio_readn(req->connfd, &chg, sizeof(int));
												while(chg > 0){
														readRequest(req, 1);
														chg--;
												}
										}else{

												while(!req->fini){
		                    	readRequest(req, 0); //On lit la requete du client
												}

										}

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

void readRequest(struct Request *req, int masterReq){

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
		free(request);
		//On recupere la requete , et remplis notre Request
		stat(req->filename, &req->sbuf);
    if(!strcmp("get", req->cmd)){
			int l = req->sbuf.st_size;
			rio_writen(req->connfd, &l, sizeof(int));
			get(req);
    }else if (!strcmp("ls", req->cmd)){
				char* args[1];
				args[0] = req->cmd;
				int c;
				int fd = open(".log", O_RDWR | O_CREAT, 0666);
				if((c=Fork())==0){
						close(1);
						dup(fd);
						execvp(args[0], args);
						exit(0);
				}
				close(fd);
				waitpid(c, NULL, 0);
				memcpy(req->filename, ".log", 4);
				stat(req->filename, &req->sbuf);
				int l = req->sbuf.st_size;
				rio_writen(req->connfd, &l, sizeof(int));
				get(req);
				remove(".log");
		}else if (!strcmp("pwd", req->cmd)){
				char* args[1];
				args[0] = "pwd";
				int c;
				int fd = open(".log", O_RDWR | O_CREAT, 0666);
				if((c=Fork())==0){
						close(1);
						dup(fd);
						execvp(args[0], args);
						exit(0);
				}
				close(fd);
				waitpid(c, NULL, 0);
				memcpy(req->filename, ".log", 4);
				stat(req->filename, &req->sbuf);
				int l = req->sbuf.st_size;
				rio_writen(req->connfd, &l, sizeof(int));
				get(req);
				remove(".log");
		}else if (!strcmp("cd", req->cmd)){
				chdir(req->filename);//On se deplace dans req->filename (qui est ici un repertoire et non pas un fichier)
		}else if (!strcmp("bye", req->cmd) ) {//Si le client envoie bye, on ferme la connexion
    	printf("fin de la connexion\n");
			Close(req->connfd);
			req->fini=1;
    }else if (!strcmp("put", req->cmd) || !strcmp("rm", req->cmd) || !strcmp("mkdir", req->cmd) ) {
		//Si c'est une des requetes causant une modification des servers
			if(!strcmp("rm", req->cmd)){
				char* args[3];
				if(!strcmp(req->filename, "-r")){
						args[0] = "rm";
						args[1] = "-r";
						args[2] = req->content;
				}else{
						args[0] = "rm";
						args[1] = req->filename;
				}
				int c;
				if((c=Fork())==0){
						execvp(args[0], args);
						exit(0);
				}
				waitpid(c, NULL, 0);
			}if(!strcmp("mkdir", req->cmd)){
				char* args[2];
				args[0] = "mkdir";
				args[1] = req->filename;
				int c;
				if((c=Fork())==0){
						execvp(args[0], args);
						exit(0);
				}
				waitpid(c, NULL, 0);

			}else{
				put(req);
			}
    	/* code */
    }else;
    fflush(stdout);
}

void get(struct Request *req){
	char buf[MAXLINE];
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
					Close(req->connfd);
					req->fini=1;
		     	fprintf(stderr, "Arret innatendu du client\n");
		     	break;
	    }
	}while (n>0);

	close(fd);

}

void put(struct Request* r) {
		int l; ssize_t n=0;
		rio_readn(req->connfd, &l, sizeof(int));
		char * buf;
		int fd = open(strcat(req->filename, "1"), O_RDWR | O_CREAT, 0666);//On creer un fichier pour le téléchargement
		buf = (char*)malloc(sizeof(char)*MAXSEND);
		int send=MAXSEND;
		if(l<MAXSEND)
			send=l;
		sleep(5);
		while((n=rio_readn(req->connfd,buf, send)) > 0 && l>0) {//Tant qu'on recoit des données, on les ecrit dans le fichier
				if (n>send)
					n=send;
				rio_writen(fd, buf, n);
				l-=n;
				if(l<MAXSEND)
					send=l;
		}
		close(fd);
}

void freeRequest(struct Request *r){
	free(r->cmd);
	free(r->filename);
	free(r->content);
	free(r);
}
