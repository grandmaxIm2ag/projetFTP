#include "client.h"
#include <sys/time.h>
#include <stdio.h>
#include "csapp.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define port 2121
#define MAX_NAME_LEN 256
#define MAXSEND 256

struct Request* req;
int main(int argc, char **argv)
{
    int clientfd, fd, port2;
    char *buf, *host, *hidefile, login[MAXLINE], password[32];
    struct timeval start, end;
    size_t len=0, n=0; int l=0;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];//On recupere le hostname du serveur (127.0.0.1 pour les tests)

    clientfd = Open_clientfd("127.0.0.1", port);//On demande la connexion au serveur maitre
    printf("client connected to server %s\n", host);


    printf("login : "); scanf("%s",login);
    printf("Password : "); scanf("%s", password);

    strcat(login, "::");
    strcat(login, password);

    rio_writen(clientfd, login, MAXLINE);//On tente de se connecer

    Rio_readn(clientfd, &port2, sizeof(int)); //On recupert le port du serveur esclave

    if(port2 == -1){
        fprintf(stderr, "L'authentification a echoue\n");
        exit(0);
    }

    printf("Vous aller être redirigé vers l'esclave %d\n", port2);

    clientfd = Open_clientfd(host, port2);//On demande la connexion au serveur esclave

    req = malloc(sizeof(struct Request));//On alloue de la memoire pour la Request
    req->clientfd = clientfd;//On recupere le descripteur du socket


    while (1) {
      printf("Tapez votre requete : \n");
      buf = (char*)malloc(sizeof(char)*MAXLINE);
      Fgets(buf, MAXLINE, stdin);
      req->filename = (char*)malloc(sizeof(char)*MAXLINE);
      req->cmd = (char*)malloc(sizeof(char)*MAXLINE);
      req->content = (char*)malloc(sizeof(char)*MAXLINE);
      sscanf(buf, "%s %s %s", req->cmd, req->filename, req->content);//On recupere la requette tapée par le client
      rio_writen(req->clientfd, buf, strlen(buf));//On envoie la requete au serveur

      /*
        Pour savoir si on a déjà tenté de télécharger un fichier, on créer un fichier temporaire au début du téléchargeme,t
        Lorsque le téléchargement est terminé on le supprime
        Si il existe un fichier temporaire associé au fichier que lon veut télécharger
        cela signifie qu'on a déjà essayer de le télécharger, on télécharge uniquement les données manquante
      */
      if(!strcmp(req->cmd, "get")){
          gettimeofday(&start, NULL);
          rio_readn(req->clientfd, &l, sizeof(int));
          hidefile = (char*)malloc(sizeof(char)*MAX_NAME_LEN);
          hidefile[0]='.';
          strcat(hidefile, req->filename);//On créer le nom du fichier temporaire
          if(stat(hidefile,&req->sbuf) != 0){//Si le fichier temporaire n'existe pas
              open(hidefile, O_RDWR | O_CREAT, 0666);//On le créer
              fd = open(strcat(req->filename, "1"), O_RDWR | O_CREAT, 0666);//On creer un fichier pour le téléchargement
              free (buf);
              buf = (char*)malloc(sizeof(char)*MAXSEND);
              int send=MAXSEND;
              if(l<MAXSEND)
                send=l;
              while((n=rio_readn(req->clientfd,buf, send)) > 0 && l>0) {//Tant qu'on recoit des données, on les ecrit dans le fichier
                  if (n>send)
                    n=send;
                  rio_writen(fd, buf, n);
                  len+=n;
                  l-=n;
                  if(l<MAXSEND)
                    send=l;
              }
              close(fd);
              gettimeofday(&end, NULL);
              double temps = ((end.tv_sec+(double)end.tv_usec/1000000)-(start.tv_sec+(double)start.tv_usec/1000000));
              printf("%lu bytes received in %f sec (%f bytes / sec) \n",len, temps, ((double)(len/temps)));//On affiche le statistiques
              if(len == 0){
                  printf("Le téléchargement a echouée\n");
                  remove(req->filename);
              }
              remove(hidefile);
          }else{//Si le fichier temporaire existe
            fd = open(strcat(req->filename, "1"), O_WRONLY);
            stat(req->filename,&req->sbuf);
            int dejaLu = req->sbuf.st_size;//On recupere la taille des données déjà téléchargé
            free (buf);
            buf = (char*)malloc(sizeof(char)*MAXSEND);
            int send = MAXSEND;
            if(l<MAXSEND)
              send=l;
            while((n=Rio_readn(req->clientfd, buf, send)) > 0) {
                int tmp = dejaLu;
                dejaLu-=n;
                if(dejaLu <= 0){//On ecrit si ce sont des données non téléchargées
                  rio_writen(fd, buf, n);
                  free(buf);
                  buf = (char*) malloc(sizeof(char)*MAXSEND);
                  len+=n;
                }else{//Sinon on déplace le curseur
                  if(n<tmp)
                      lseek(fd, n, SEEK_CUR);
                  else
                      lseek(fd, n-tmp, SEEK_CUR);
                }
                l-=n;
                if(l<MAXSEND)
                  send=l;

            }
            close(fd);
            gettimeofday(&end, NULL);
            double temps = ((end.tv_sec+(double)end.tv_usec/1000000)-(start.tv_sec+(double)start.tv_usec/1000000));
            printf("%lu bytes received in %f sec (%f bytes / sec) \n",len, temps, ((double)(len/temps)));
            remove(hidefile);
          }

      }else if(!strcmp(req->cmd, "bye")){//On ferme la connexion
          Rio_writen(req->clientfd, req->cmd, strlen(req->cmd));
          printf("fin de la connexion\n");
          exit(0);
      }else if(!strcmp("ls", req->cmd) || !strcmp("pwd", req->cmd)){
        int send=MAXSEND;
        buf = (char*)malloc(sizeof(char)*MAXSEND);
        rio_readn(req->clientfd, &l, sizeof(int));
        if(l<MAXSEND)
          send=l;
        while((n=rio_readn(req->clientfd,buf, send)) > 0 && l>0) {//Tant qu'on recoit des données, on les ecrit dans le fichier
            if (n>send)
              n=send;
            fflush(stdout);
            l-=n;
            if(l<MAXSEND)
              send=l;
        }
  		}else if (!strcmp("put", req->cmd)){
  		  //Si c'est une des requetes causant une modification des servers
        stat(req->filename, &req->sbuf);
        int l = req->sbuf.st_size;
        rio_writen(req->clientfd, &l, sizeof(int));
        int fd = open(req->filename, O_RDONLY);
        /*
        Tant qu'on a ecris, on continue d'ecrire
        */
        do {
            n = rio_readn(fd, buf, MAXSEND);
            rio_writen(req->clientfd, buf, n);
        }while (n>0);

        close(fd);
      }else if(!strcmp("cd", req->cmd)){}
      else if(!strcmp("rm", req->cmd) || !strcmp("mkdir", req->cmd)){}
      else{
          printf("Commande %s inconnue\n", req->cmd);
      }
    }

}

void freeRequest(struct Request *r){
	free(r->cmd);
	free(r->filename);
	free(r->content);
	free(r);
}
