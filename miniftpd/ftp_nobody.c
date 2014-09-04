#include "ftp_nobody.h"
#include "sysutil.h"
#include "common.h"
#include "priv_sock.h"
#include "priv_command.h"

int capset(cap_user_header_t hdrp, const cap_user_data_t datap);
void set_bind_capabilities();
void set_nobody()
{
    //基本思路
    //1.首先获取nobody的uid、gid
    //2.然后逐项进行设置
    struct passwd *pw;
    if((pw = getpwnam("nobody")) == NULL)
        ERR_EXIT("getpwnam");

    if(setegid(pw->pw_gid) == -1)
        ERR_EXIT("setegid");
    
    if(seteuid(pw->pw_uid) == -1)
        ERR_EXIT("seteuid");
    
}
void handle_nobody(session_t *sess)
{
    set_nobody();
    set_bind_capabilities();
/*modify to priv
 * char cmd[1024];
    while(1)
    {
    int ret = readn(sess->nobody_fd,cmd,1024);
    if(ret == -1)
        {
            if(errno == EINTR)
                continue;
            ERR_EXIT("readn");
        }
    while(1)
    {
        pause();
    }
    */
    char cmd;
    while(1)
    {
        cmd = priv_sock_recv_cmd(sess->nobody_fd);
        switch (cmd)
        {
    
            case PRIV_SOCK_GET_DATA_SOCK:
                privop_pasv_get_data_sock(sess);
                break;
            case PRIV_SOCK_PASV_ACTIVE:
                privop_pasv_active(sess);
                break;
            case PRIV_SOCK_PASV_LISTEN:
                privop_pasv_listen(sess);
                break;
            case PRIV_SOCK_PASV_ACCEPT:
                privop_pasv_accept(sess);
                break;
            default:
                fprintf(stderr,"Unknow command\n");
                exit(EXIT_FAILURE);

        }
    }
       
}

void set_bind_capabilities()
{
   struct __user_cap_header_struct cap_user_header;
    cap_user_header.version = _LINUX_CAPABILITY_VERSION_1;
    cap_user_header.pid = getpid();

    struct __user_cap_data_struct cap_user_data;
    __u32 cap_mask = 0; //类似于权限的集合
    cap_mask |= (1 << CAP_NET_BIND_SERVICE); //0001000000
    cap_user_data.effective = cap_mask;
    cap_user_data.permitted = cap_mask;
    cap_user_data.inheritable = 0; //子进程不继承特权

    if(capset(&cap_user_header, &cap_user_data) == -1)
        ERR_EXIT("capset");  
}

int capset(cap_user_header_t hdrp, const cap_user_data_t datap)
{
    return syscall(SYS_capset, hdrp, datap);
}
