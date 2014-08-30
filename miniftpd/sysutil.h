#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include "common.h"
#define ERR_EXIT(m) \
    do { \
        perror(m);\
        exit(EXIT_FAILURE);\
    }while(0)
    int tcp_server(const char *host, unsigned short port);
    int getlocalip(char *ip);
    void activate_nonblock(int fd);
    void deactivate_nonblock(int fd);
    int read_timeout(int fd, unsigned int wait_seconds);
    int write_timeout(int fd, unsigned int wait_seconds);
    int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds);
    int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds);
    ssize_t readn(int fd, void *buf, size_t count);
    ssize_t writen(int fd, const void *buf, size_t count);
    ssize_t recv_peek(int sockfd, void *buf, size_t len);
    ssize_t readline(int sockfd, void *buf, size_t maxline);
    void send_fd(int sock_fd, int fd);
    int recv_fd(const int sock_fd);
