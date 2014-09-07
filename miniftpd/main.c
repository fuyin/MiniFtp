#include "common.h"
#include "sysutil.h"
#include "strutil.h"
#include "session.h"
#include "parse_conf.h"
#include "configure.h"
#include "trans_ctrl.h"
#include "hash.h"
#include "ftp_code.h"
#include "command_map.h"
#include "ftp_assist.h"

static void do_root();
extern int clientcount;
static void  server_init();
static void server_begin();

extern session_t *p_sess;
hash_t *ip_to_clients;
hash_t *pid_to_ip;
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

    setup_signal_chld();
    //signal(SIGCHLD,handle_chld);
    //signal(SIGPIPE,SIG_IGN);

    do_root();

    parse_conf_load_file("ftp.conf");

    init_hash();    
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
    p_sess=&sess;
    

    while(1)
    {
    struct sockaddr_in addr;
        int peerfd=accept_timeout(listenfd,&addr,tunable_accept_timeout);
        
        uint32_t ip=addr.sin_addr.s_addr;
        add_clients_to_hash(&sess, ip);

        if(peerfd == -1 && errno == ETIMEDOUT)
            continue;
        else if(peerfd==-1)
            ERR_EXIT("accept_timeout");
    
        printf("clientcount =%d\n",++clientcount);
        sess.curr_clients =clientcount;
        // fork ,child process do session(nobody,proto)
        if((pid = fork())==0)
        {
            close(listenfd);
            // process session
            sess.peerfd=peerfd;
            limit_num_clients(&sess);
            session_begin(&sess);

        
        }
        else if(pid>0)
        {
              add_pid_ip_to_hash(pid, ip);
            close(peerfd);
            printf("main\n");
        }
    
    }
}

