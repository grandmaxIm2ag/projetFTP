#include "client.h"
#include <sys/time.h>
#include <stdio.h>
#include "csapp.h"
#include <stdlib.h>
#include <string.h>

#define port 2121

int main(int argc, char **argv)
{
    int clientfd, fd, fderr, lenerr=0;
    char *host, buf[MAXLINE], filename[MAXLINE], str1[MAXLINE];
    rio_t rio;
    struct timeval start, end;
    size_t len=0, n=0;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];

    /*
     * Note that the 'host' can be a name or an IP address.
     * If necessary, Open_clientfd will perform the name resolution
     * to obtain the IP address.
     */
    clientfd = Open_clientfd(host, port);
    
    /*
     * At this stage, the connection is established between the client
     * and the server OS ... but it is possible that the server application
     * has not yet called "Accept" for this connection
     */
    printf("client connected to server OS\n"); 
    
    Rio_readinitb(&rio, clientfd);

    Fgets(buf, MAXLINE, stdin);
    gettimeofday(&start, NULL);
    Rio_writen(clientfd, buf, strlen(buf));
    
     
    printf("Ici ?\n");    
    
    
    sscanf(buf, "%s %s %s", str1, filename, str1);
    
    fd = open(strcat(filename, "Copy"), O_RDWR | O_CREAT, 0666);
    fderr = open(strcat(".", filename),  O_RDWR | O_CREAT, 0666);
    printf("ouvert : %s\n",filename);
    printf("%d\n",fd);

    while((n=Rio_readn(clientfd, buf, 100)) > 0) {
        printf("%d\n", n);
        rio_writen(fd, buf, strlen(buf));
        len+=n;
        fdatasync(fd);
    }

    close(fd);
    printf("fermé\n");

    
    gettimeofday(&end, NULL);
    long int temps = ((end.tv_sec+end.tv_usec/1000000)-(start.tv_sec+start.tv_usec/1000000));
    printf("%lu bytes received in %ld sec\n ",len, temps);
    Close(clientfd);
    exit(0);
}
