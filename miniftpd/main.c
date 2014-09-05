#include "common.h"
#include "sysutil.h"
#include "strutil.h"
#include "session.h"
#include "parse_conf.h"
#include "configure.h"
static void do_root();
extern int clientcount;
static void  server_init();
static void server_begin();
static void handle_chld();
int main(int argc, const char *argv[])
{
    server_init();
    server_begin();
    return 0;
}
static void do_root()
{
    if(getuid())
    {
        fprintf(stderr,"miniftp must be started by root\n");
        exit(EXIT_FAILURE);
    }
}

static void  server_init()
{

    signal(SIGCHLD,handle_chld);
    signal(SIGPIPE,SIG_IGN);

    do_root();

    parse_conf_load_file("ftp.conf");

    //create a listenfd
      printf("local(listen) address %s\n",tunable_listen_address);
}

static void server_begin()
{
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
}

static void handle_chld()
{
    while(waitpid(-1,NULL,WNOHANG)>0){}
}
