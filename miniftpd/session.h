#ifndef SESION_H
#define SESION_H 
#include "common.h"

typedef struct
{
    int peerfd;//客户连接的fd

    int nobody_fd;//nobody进程的fd
    int proto_fd;//孙子进程的fd

    char command[1024];//客户端发来的指令
    char comm[512];//FTP指令
    char args[512];//FTP参数

    uid_t user_uid; //用户的uid
    int ascii_mode; //是否为ascii传输模式
}session_t;
 int clientcount;
void session_init(session_t *sess);

void session_begin(session_t *sess);

#endif  /*SESION_H*/
