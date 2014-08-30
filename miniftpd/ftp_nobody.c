#include "ftp_nobody.h"
#include "sysutil.h"
#include "common.h"
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
    char cmd[1024];
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
    }   
}
