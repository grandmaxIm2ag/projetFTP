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

int slave[NB_PROC];

int next(){//Renvoie le prochain esclave
		int p = slave[sCourant];
		sCourant = ++sCourant % NB_PROC;
		return p;
}
int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];
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
					int p = next();
					Rio_writen(connfd, &p, sizeof(p));//Envoie au client le numero de port de l'esclve traitant

          Close(connfd);//Ferme la connexion avec le client
      }
    }

    exit(0);
}
