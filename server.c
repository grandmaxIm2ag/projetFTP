#include "server.h"

#include "csapp.h"

#define MAX_NAME_LEN 256
#define NB_PROC 10
#define port 2121 

pid_t child[NB_PROC];

int main(int argc, char **argv)
{
    Signal(SIGINT, stop); 
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
                    Getnameinfo((SA *) &clientaddr, clientlen,
                                client_hostname, MAX_NAME_LEN, 0, 0, 0);
                    
                    
                    /* determine the textual representation of the client's IP address */
                    Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                            INET_ADDRSTRLEN);
                    
                    printf("server connected to %s (%s) %d\n", client_hostname,
                        client_ip_string, getpid());
                    
                    echo(connfd);
                    Close(connfd);
                }
            }
            exit(0);
        }
        child[i] = p;
    }
    
    for(int i=0; i<NB_PROC; i++)
        waitpid(child[i], NULL, 0);
    
    exit(0);
}