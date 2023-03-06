#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";


void proxy(int fd);
int parse_uri(char *uri, char* domain, char* port, char *path);
int read_parse_hdrs(rio_t *rp, char* domain, char* headers);
int connect_server(char* host, char* port, char* buf, int webfd);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2){
        fprintf(stderr, "usage: %s <port>\n",argv[0]);
        exit(1);
    }

    /* Listen */
    listenfd = Open_listenfd(argv[1]);

    /* Main loop */
    while(1){
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);   //Accept
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE,
                    port, MAXLINE, 0);
        printf("Accepted connection form (%s, %s)\n", hostname, port);
        proxy(connfd);
        Close(connfd);

    }


    printf("End\n");
    return 0;
}

void proxy(int fd){
    rio_t rio;
    char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
    char domain[MAXLINE],port[MAXLINE],path[MAXLINE],headers[MAXLINE];
    /* Read request line*/
    Rio_readinitb(&rio, fd);
    if(!Rio_readlineb(&rio, buf, MAXLINE)){
        printf("Proxy::Can't read request!\n");
        return;
    }
    printf("%s",buf);

    /*Deal with request line */
    sscanf(buf, "%s %s %s", method, uri, version);

    if(strcasecmp(method, "GET")){
        printf("Proxy::Method is not GET\n");
        //return;
    }

    parse_uri(uri,domain,port,path);

    /* build new request line */
    sprintf(buf,"%s %s HTTP/1.0\r\n",method,path);
    printf("New request: %s\n", buf);

    /* Read and parse request headers */
    read_parse_hdrs(&rio, domain, headers);
    sprintf(buf,"%s%s",buf,headers);

    /* Connect to server, get response */
    connect_server(domain, port, buf, fd);

    /* Send response back to client */
    //Rio_writen(fd, rebuf, strlen(rebuf));

}

int parse_uri(char *uri, char* domain, char* port, char *path){
    char *start = strstr(uri,"//");

    if(start){
        start += 2; // "//" occupies two characters
        char* mid = strchr(start, ':');
        if(mid){
            strncpy(domain, start, mid-start);
            domain[mid-start] = '\0';
            
            mid += 1; //":" occupies one characters
            char* end = strchr(mid, '/');
            if(end){
                strncpy(port, mid, end-mid);
                port[end-mid] = '\0';
                strncpy(path, end, MAXLINE);
            }
            else{
                path[0] = '/';
                path[1] = '\0';
            }            

        }
        else{
            sprintf(port,"80");
            char* end = strchr(start, '/');
            if(end){
                strncpy(domain, start, end-start);
                domain[end-start] = '\0';
                strncpy(path, end, MAXLINE);
            }
            else{
                strcpy(domain, start);
                domain[strlen(start)] = '\0';
                path[0] = '/';
                path[1] = '\0';
            }
        }
        printf("Host:%s Port:%s Path:%s\n", domain, port, path);
        return 0;
    }
    else{
        return -1;
    }
}

int read_parse_hdrs(rio_t *rp, char* domain, char* headers){
    char line[MAXLINE],head[MAXLINE],remain[MAXLINE];

    Rio_readlineb(rp, line, MAXLINE);
    printf("%s", line);
    sscanf(line,"%s %s",head,remain);
    if(strcmp(head,"Host:")){   //No Host
        sprintf(headers,"Host: %s\r\n",domain);
    }
    else{   //already have Host
        sprintf(headers, "%s", line);
    }

    //add User-Agent
    sprintf(headers,"%s%s",headers,user_agent_hdr);
    //add Connection
    sprintf(headers,"%sConnection: close\r\n",headers);
    //add Proxy-Connection
    sprintf(headers,"%sProxy-Connection: close\r\n",headers);

    while(strcmp(line,"\r\n")){
        Rio_readlineb(rp, line, MAXLINE);
        printf("%s", line);
        sscanf(line,"%s %s",head,remain);
        if(!strcmp(head,"User-Agent:") || !strcmp(head,"Connection:") || !strcmp(head,"Proxy-Connection:")){
            ;
        }
        else{
            sprintf(headers,"%s%s",headers,line);
        }
    }

    sprintf(headers,"%s\r\n",headers);
    return 0;
}

int connect_server(char* host, char* port, char* buf, int webfd){
    int clientfd;
    rio_t rio;
    char line[MAXLINE];
    size_t n;

    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    Rio_writen(clientfd, buf, MAXLINE);
    while((n = Rio_readlineb(&rio, line, MAXLINE)) != 0){
        printf("server received %d bytes\n", (int)n);
        Rio_writen(webfd, line, n);
    }

    Close(clientfd);

    return 0;
}