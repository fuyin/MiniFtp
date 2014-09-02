#include "common.h"
#include "sysutil.h"
#include "strutil.h"
#include "session.h"
#include "parse_conf.h"
#include "configure.h"
void do_root();
extern int clientcount;
void handle_chld()
{
    while(waitpid(-1,NULL,WNOHANG)>0){}
}
int main(int argc, const char *argv[])
{
    signal(SIGCHLD,handle_chld);
    signal(SIGPIPE,SIG_IGN);

    do_root();

    parse_conf_load_file("ftp.conf");
    //print_conf();

    //create a listenfd
      printf("local(listen) address %s\n",tunable_listen_address);
    int listenfd=tcp_server(tunable_listen_address,tunable_listen_port);

    int pid;
    session_t sess;
     session_init(&sess);
     clientcount=0;
    while(1)
    {
        int peerfd=accept_timeout(listenfd,NULL,tunable_accept_timeout);

        if(peerfd == -1 && errno == ETIMEDOUT)
            continue;
        else if(peerfd==-1)
            ERR_EXIT("accept_timeout");
        printf("clientcount =%d\n",++clientcount);
        // fork ,child process do session(nobody,proto)
        if((pid = fork())==0)
        {
            close(listenfd);
            // process session
            sess.peerfd=peerfd;
            session_begin(&sess);
        
        }
        else if(pid>0)
        {
            close(peerfd);
            printf("main\n");
        }
    
    }
    return 0;
}
void do_root()
{
    if(getuid())
    {
        fprintf(stderr,"miniftp must be started by root\n");
        exit(EXIT_FAILURE);
    }
}
