#include "session.h"
#include "ftp_nobody.h"
#include "ftp_proto.h"
void session_init(session_t *sess)
{
    sess->peerfd = -1;
    sess->nobody_fd = -1;
    sess->proto_fd = -1;
    sess->user_uid=0;
    sess->ascii_mode=0;
    memset(sess->command,0x00,sizeof(sess->command));
    memset(sess->comm,0x00,sizeof(sess->comm));
    memset(sess->args,0x00,sizeof(sess->args));
    sess->p_addr=NULL;
   sess->data_fd = -1;
   sess->listen_fd = -1;
}

void session_begin(session_t *sess)
{
    //build PCI   socketpair
    int fds[2];
    if(socketpair(PF_UNIX,SOCK_STREAM,0,fds)==-1)
        ERR_EXIT("socketpair");
    
    // fork nobody and proto
    int pid;
    if((pid=fork())==0)
    {
        printf("client %d proto\n",clientcount);
        close(fds[0]);
        sess->proto_fd= fds[1];
        //handle proto
        handle_proto(sess);
    }
    else if(pid > 0)
    {
        printf("client %d nobody\n",clientcount);
        close(fds[1]);
        sess->nobody_fd=fds[0];
        //handle nobody
        handle_nobody(sess);
    }
}
