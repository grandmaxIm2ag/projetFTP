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

rio_t rio;
struct Request* req;
int main(int argc, char **argv)
{
    int clientfd, listenfd, fd;
    char *buf, *host, *hidefile;
    struct timeval start, end;
    size_t len=0, n=0;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];



    clientfd = Open_clientfd(host, port);
    req = malloc(sizeof(struct Request));
    req->clientfd = clientfd;
    Rio_readinitb(&rio, clientfd);

    printf("client connected to server %s\n", host);

    while (1) {
      printf("Tapez votre requete : \n");
      buf = (char*)malloc(sizeof(char)*MAXLINE);
      Fgets(buf, MAXLINE, stdin);
      req->filename = (char*)malloc(sizeof(char)*MAXLINE);
      req->cmd = (char*)malloc(sizeof(char)*MAXLINE);
      req->content = (char*)malloc(sizeof(char)*MAXLINE);
      sscanf(buf, "%s %s %s", req->cmd, req->filename, req->content);

      if(!strcmp(req->cmd, "get")){
          gettimeofday(&start, NULL);
          Rio_writen(req->clientfd, buf, strlen(buf));
          Close(clientfd);
          listenfd = Open_listenfd(3000);
          while((req->clientfd = Accept(listenfd, (SA *)&clientaddr, &clientlen)) < 0);

          /* determine the name of the client */
          Getnameinfo((SA *) &clientaddr, clientlen,client_hostname, MAX_NAME_LEN, 0, 0, 0);

          /* determine the textual representation of the client's IP address */
          Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,INET_ADDRSTRLEN);

          hidefile = (char*)malloc(sizeof(char)*MAX_NAME_LEN);
          hidefile[0]='T';
          strcat(hidefile, req->filename);
          if(stat(hidefile,&req->sbuf) != 0){
              open(hidefile, O_RDWR | O_CREAT, 0666);
              fd = open(strcat(req->filename, "1"), O_RDWR | O_CREAT, 0666);
              free (buf);
              buf = (char*)malloc(sizeof(char)*MAXSEND);
              while((n=Rio_readn(req->clientfd, buf, MAXSEND)) > 0) {
                  rio_writen(fd, buf, n);
                  len+=n;
                  free(buf);
                  buf = (char*) malloc(sizeof(char)*MAXSEND);
              }
              Close(fd);
              gettimeofday(&end, NULL);
              double temps = ((end.tv_sec+(double)end.tv_usec/1000000)-(start.tv_sec+(double)start.tv_usec/1000000));
              printf("%lu bytes received in %f sec (%f bytes / sec) \n",len, temps, ((double)(len/temps)));
              if(len == 0){
                  printf("Le téléchargement a echoue\n");
                  remove(req->filename);
              }
              remove(hidefile);
          }else{
            fd = open(strcat(req->filename, "1"), O_WRONLY);
            stat(req->filename,&req->sbuf);
            int dejaLu = req->sbuf.st_size;
            free (buf);
            buf = (char*)malloc(sizeof(char)*MAXSEND);
            while((n=Rio_readn(req->clientfd, buf, MAXSEND)) > 0) {
                int tmp = dejaLu;
                dejaLu-=n;
                printf("%d\n", dejaLu);
                if(dejaLu <= 0){
                  rio_writen(fd, buf, n);
                  free(buf);
                  buf = (char*) malloc(sizeof(char)*MAXSEND);
                  len+=n;
                }else{
                  if(n<tmp)
                      lseek(fd, n, SEEK_CUR);
                  else
                      lseek(fd, n-tmp, SEEK_CUR);
                }
            }
            gettimeofday(&end, NULL);
            double temps = ((end.tv_sec+(double)end.tv_usec/1000000)-(start.tv_sec+(double)start.tv_usec/1000000));
            printf("%lu bytes received in %f sec (%f bytes / sec) \n",len, temps, ((double)(len/temps)));
            remove(hidefile);
          }
      }else if(!strcmp(req->cmd, "bye")){
          Rio_writen(req->clientfd, req->cmd, strlen(req->cmd));
          printf("fin de la connexion\n");
          exit(0);
      }else{
          printf("Commande %s inconnue\n", req->cmd);
      }
    }

}
