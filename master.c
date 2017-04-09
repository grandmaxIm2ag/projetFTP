#include "server.h"
#include <string.h>
#include <stdio.h>
#include "csapp.h"
#include <stdlib.h>
#define MAX_NAME_LEN 256
#define MAXSEND 5
#define NB_PROC 1
#define port 2121


int sCourant = 0;
char ** modifServ;
int chg=0;
int slave[NB_PROC];

int next(){//Renvoie le prochain esclave
		int p = slave[sCourant];
		sCourant = ++sCourant % NB_PROC;
		return p;
}
int main(int argc, char **argv)
{
    int listenfd, connfd, p;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN], login[MAXLINE];
		rio_t rio;

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


          printf("server connected to %s (%s) %d\n", client_hostname,client_ip_string, getpid());


					Rio_readinitb(&rio, connfd);

					size_t n = Rio_readlineb(&rio, login, MAXLINE);



					if(!strcmp(login, "slave::slave")){//Si c'est unserveur esclave qui se connecte
							rio_readn(connfd, &chg, sizeof(int));//On récupert le nombre de changement a faire

							struct Request * req = malloc(sizeof(struct Request));
							req->connfd = connfd;
							modifServ = (char **)malloc(sizeof(char *)*chg);
							getsockname(req->connfd, (SA *) &clientaddr, &clientlen);
							p = ntohs(clientaddr.sin_port);
							while(chg>0){
									readRequest(req,0);//On fait tous les changements dans le repertoire de master
							}
							for(int i=0; i<NB_PROC; i++){
									if(slave[i]!=p){//On met a jours les repertoires de tous les esclaves excepte celui deja traite
											req->connfd = Open_clientfd("127.0.0.1", slave[i]);
											rio_writen(req->connfd, &chg, sizeof(int));
											int chg2 = chg;
											while(chg2>=0){
													char* buf;
													chg2--;
													req->filename = (char*)malloc(sizeof(char)*MAXLINE);
													req->cmd = (char*)malloc(sizeof(char)*MAXLINE);
													req->content = (char*)malloc(sizeof(char)*MAXLINE);
													sscanf(modifServ[chg2], "%s %s %s", req->cmd, req->filename, req->content);//On recupere la requette tapée par le client
													rio_writen(req->connfd, modifServ[chg2], strlen(modifServ[chg2]));//On envoie la requete au serveur

													if(!strcmp("cd", req->cmd));
													else if (!strcmp("put", req->cmd)){
															//Si c'est une des requetes causant une modification des servers
															stat(req->filename, &req->sbuf);
															int l = req->sbuf.st_size;
															rio_writen(req->connfd, &l, sizeof(int));
															int fd = open(req->filename, O_RDONLY);
															buf = (char *)malloc(sizeof(char)*MAXLINE);
															/*
															Tant qu'on a ecris, on continue d'ecrire
															*/
															do {
																	n = rio_readn(fd, buf, MAXSEND);
																	rio_writen(req->connfd, buf, n);
															}while (n>0);
															close(fd);
													}else if(!strcmp("rm", req->cmd) || !strcmp("mkdir", req->cmd));
													else{
															printf("Commande %s inconnue\n", req->cmd);
													}
											}
								  }
						  }
					}else{
							login[n-1]='\0';
							int b = auth(login);

							if(b)
								 p = next();
							else
								p= -1;

							Rio_writen(connfd, &p, sizeof(p));//Envoie au client le numero de port de l'esclave traitant
		          Close(connfd);//Ferme la connexion avec le client
							
					}
      }
    }

    exit(0);
}

int auth(char * login){
	int b=0;
	char buf[MAXLINE];

	int fd = open(".login", O_RDONLY);//On ouvre le fichier comportant tous les login::passxord
	rio_t rio;
	Rio_readinitb(&rio, fd);

	size_t n =0;

	while((n = Rio_readlineb(&rio, buf, MAXLINE))>0 && !b){
			//On lit chacune des lignes
			//Si on lit le login envoye par le clientfd
			//b prend la valeur 1 et on arrete la lecture
			buf[n-1]='\0';
			if(!strcmp(buf, login)){
					b=1;
					break;
			}
	}

	return b;
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
		//On recupere la requete , et remplis notre Request
		stat(req->filename, &req->sbuf);
    if (!strcmp("cd", req->cmd)){
				modifServ[chg] = (char *)malloc(sizeof(char)*strlen(request));
				memcpy(modifServ, request, strlen(request));
				chg++;
				chdir(req->filename);//On se deplace dans req->filename (qui est ici un repertoire et non pas un fichier)
		}else if (!strcmp("put", req->cmd) || !strcmp("rm", req->cmd) || !strcmp("mkdir", req->cmd) ) {
		//Si c'est une des requetes causant une modification des servers
			if(!masterReq){
					modifServ[chg] = (char *)malloc(sizeof(char)*strlen(request));
					memcpy(modifServ, request, strlen(request));
					chg++;
			}
			if(!strcmp("rm", req->cmd)){
				char* args[3]; int f;
				if(strcmp(req->filename, "-r")){
						args[0] = "rm";
						args[1] = req->filename;
						f =1;
				}else{
						args[0] = "rm";
						args[1] = "-r";
						args[2] = req->content;
						f=2;
				}
				int c;
				if((c=Fork())==0){
						if(stat(args[f],&req->sbuf) == 0)
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

			}
    	/* code */
    }else;
    fflush(stdout);
}


void put(struct Request* req) {
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
