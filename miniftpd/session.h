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

    struct sockaddr_in *p_addr;//port模式下对方的ip和port
    int data_fd;//数据传输fd

    int listen_fd;//PASV被动模式下监听的fd

    uint64_t restart_pos; //文件传输断点

    char * rnfr_name;  //文件重命名 RNTR RNTO
}session_t;
 int clientcount;
void session_init(session_t *sess);

void session_begin(session_t *sess);

#endif  /*SESION_H*/
