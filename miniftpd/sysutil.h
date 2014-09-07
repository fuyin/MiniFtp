#ifndef SYSUTIL_H
#define SYSUTIL_H 

#include "common.h"
#define ERR_EXIT(m) \
    do { \
        perror(m);\
        exit(EXIT_FAILURE);\
    }while(0)
    int tcp_client(unsigned int port);
    int tcp_server(const char *host, unsigned short port);
    int get_local_ip(char *ip);
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
    
int lock_file_read(int fd);
int lock_file_write(int fd);
int unlock_file(int fd);

int get_curr_time_sec();
int get_curr_time_usec();
int nano_sleep(double t);
#endif  /*SYSUTIL_H*/
