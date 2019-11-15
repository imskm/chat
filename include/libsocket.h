#ifndef __LIBSOCKET_H
#define __LIBSOCKET_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

#define MAXLINE         1024
#define BUFFSIZE        1024
#define LISTENQ         8
#define SERVER_IP       "127.0.0.1"
#define SERVER_PORT     3333

#define max(a, b)		(((a) > (b)) ? (a) : (b))

int Socket(int domain, int type, int protocol);
int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Listen(int sockfd, int backlog);
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int Inet_pton(int af, const char *src, void *dst);

#endif	/* LIBSOCKET end */
