#include "priv_command.h"
#include "priv_sock.h"
#include "common.h"
#include "configure.h"
#include "parse_conf.h"
#include "sysutil.h"

void privop_pasv_get_data_sock(session_t *sess)
{
    char ip[16]={0};
    priv_sock_recv_str(sess->nobody_fd,ip,sizeof(ip));
    uint16_t port = priv_sock_recv_int(sess->nobody_fd);

    int data_fd=tcp_client(0);

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=inet_addr(ip);
    //connect
    int ret = connect_timeout(data_fd,&addr,tunable_data_connection_timeout);
    if(ret == -1)
        ERR_EXIT("connect_timeout");
    priv_sock_send_result(sess->nobody_fd,PRIV_SOCK_RESULT_OK);
    priv_sock_send_fd(sess->nobody_fd,data_fd);
    close(data_fd);
    
}

void privop_pasv_active(session_t *sess)
{
    priv_sock_send_int(sess->nobody_fd,(sess->listen_fd!=-1));
}


void privop_pasv_listen(session_t *sess)
{
    char ip[16];
    get_local_ip(ip);
    int listen_fd=tcp_server(ip,20);
    sess->listen_fd=listen_fd;
    
    struct sockaddr_in addr;

    socklen_t len= sizeof addr;
    if(getsockname(listen_fd,(struct sockaddr *)&addr,&len)==-1)
    {
        fprintf(stderr,"gethostname");
        return;
    }
    priv_sock_send_result(sess->nobody_fd,PRIV_SOCK_RESULT_OK);

    uint16_t port=ntohs(addr.sin_port);
    //    printf("%d\n",port);
    priv_sock_send_int(sess->nobody_fd,port);
}

void privop_pasv_accept(session_t *sess)
{
    //接受新连接
    int peerfd = accept_timeout(sess->listen_fd, NULL, tunable_accept_timeout);
    if(peerfd == -1)
    {
        priv_sock_send_result(sess->nobody_fd, PRIV_SOCK_RESULT_BAD);
        ERR_EXIT("accept_timeout");
    }
    
    //给对方回应
    priv_sock_send_result(sess->nobody_fd, PRIV_SOCK_RESULT_OK);

    //将data fd传给对方
    priv_sock_send_fd(sess->nobody_fd, peerfd);
    close(peerfd);
}

