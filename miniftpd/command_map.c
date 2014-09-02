#include "command_map.h"
#include "trans_data.h"
#include "sysutil.h"
#include "ftp_code.h"
#include "strutil.h"
#include "common.h"
#include "configure.h"
typedef void (*Func)(session_t *);
typedef struct command_map
{
    const char *command;
    Func cmd_handler;
}ftpcmd_t;
static ftpcmd_t ctrl_cmds[]=
{
    /* 访问控制命令 */
    {"USER",    do_user },
    {"PASS",    do_pass },
    {"CWD",     do_cwd  },
    {"XCWD",    do_cwd  },
    {"CDUP",    do_cdup },
    {"XCUP",    do_cdup },
    {"QUIT",    do_quit },
    {"ACCT",    NULL    },
    {"SMNT",    NULL    },
    {"REIN",    NULL    },
    /* 传输参数命令 */
    {"PORT",    do_port },
    {"PASV",    do_pasv },
    {"TYPE",    do_type },
    {"STRU",    do_stru },
    {"MODE",    do_mode },

    /* 服务命令 */
    {"RETR",    do_retr },
    {"STOR",    do_stor },
    {"APPE",    do_appe },
    {"LIST",    do_list },
    {"NLST",    do_nlst },
    {"REST",    do_rest },
    {"ABOR",    do_abor },
    {"\377\364\377\362ABOR", do_abor},
    {"PWD",     do_pwd  },
    {"XPWD",    do_pwd  },
    {"MKD",     do_mkd  },
    {"XMKD",    do_mkd  },
    {"RMD",     do_rmd  },
    {"XRMD",    do_rmd  },
    {"DELE",    do_dele },
    {"RNFR",    do_rnfr },
    {"RNTO",    do_rnto },
    {"SITE",    do_site },
    {"SYST",    do_syst },
    {"FEAT",    do_feat },
    {"SIZE",    do_size },
    {"STAT",    do_stat },
    {"NOOP",    do_noop },
    {"HELP",    do_help },
    {"STOU",    NULL    },
    {"ALLO",    NULL    }
};
void do_command_map(session_t *sess)
{
    int i;
    int size = sizeof(ctrl_cmds)/sizeof(ctrl_cmds[0]);
    for(i=0;i!=size;i++)
    {
            str_upper(sess->comm);
        if(strcmp(ctrl_cmds[i].command,sess->comm)==0)
        {
            #ifdef DEBUG
            printf("ctrl_cmds[i].command %s\n sess->comm %s\n",ctrl_cmds[i].command,sess->comm);
            #endif
            if(ctrl_cmds[i].cmd_handler != NULL)
                ctrl_cmds[i].cmd_handler(sess);
            else
            {
                //命令没有实现
                ftp_reply(sess,FTP_COMMANDNOTIMPL, "Unimplement command.");
            }
            break;
        }
    }
    if(i==size)
    {
        ftp_reply(sess,FTP_BADCMD,"Unknow command.");
    }

}
void ftp_reply(session_t *sess, int status, const char *text)
{
    char tmp[1024] = { 0 };
    snprintf(tmp, sizeof tmp, "%d %s\r\n", status, text);
    writen(sess->peerfd, tmp, strlen(tmp));
}

void do_user(session_t *sess)
{
    struct passwd *pw;
    if((pw = getpwnam(sess->args)) == NULL)
    {
        ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
        return;
    }

    sess->user_uid = pw->pw_uid;
    ftp_reply(sess, FTP_GIVEPWORD, "Please specify the password.");
}

void do_pass(session_t *sess)
{
    //struct passwd *getpwuid(uid_t uid)
    struct passwd *pw;
    if((pw = getpwuid(sess->user_uid)) == NULL)
    {
        ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
        return;
    }

    //struct spwd *getspnam(const char *name);
    struct spwd *spw;
    if((spw = getspnam(pw->pw_name)) == NULL)
    {
        ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
        return;
    }

    //ar *crypt(const char *key, const char *salt);
    char *encrypted_password = crypt(sess->args, spw->sp_pwdp);
    if(strcmp(encrypted_password, spw->sp_pwdp) != 0)
    {
        ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
        return;
    }

    if(setegid(pw->pw_gid) == -1)
        ERR_EXIT("setegid");
    if(seteuid(pw->pw_uid) == -1)
        ERR_EXIT("seteuid");

    //home
    if(chdir(pw->pw_dir) == -1)
        ERR_EXIT("chdir");
    //umask
    umask(tunable_local_umask);

    ftp_reply(sess, FTP_LOGINOK, "Login successful.");
    chdir("sess->args");
}


void do_cwd(session_t *sess)
{

}

void do_cdup(session_t *sess)
{

}

void do_quit(session_t *sess)
{

}

void do_port(session_t *sess)
{
    //设置主动工作模式 
    //PORT 192,168,44,1,200,174
    unsigned int v[6]={0};
    sscanf(sess->args,"%u,%u,%u,%u,%u,%u",&v[0],&v[1],&v[2],&v[3],&v[4],&v[5]);

    sess->p_addr = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
    memset(sess->p_addr,0,sizeof(struct sockaddr_in));
    sess->p_addr->sin_family =AF_INET;

    char *p=(char*)&sess->p_addr->sin_port;
    p[0]=v[4];
    p[1]=v[5];

    p=(char *)&sess->p_addr->sin_addr.s_addr;
    p[0]=v[0];
    p[1]=v[1];
    p[2]=v[2];
    p[3]=v[3];

    ftp_reply(sess,FTP_PORTOK,"PORT command sucessful");



}

void do_pasv(session_t *sess)
{

}

void do_type(session_t *sess)
{
    //指定FTP的传输模式
    if (strcmp(sess->args, "A") == 0)
    {
        sess->ascii_mode = 1;
        ftp_reply(sess, FTP_TYPEOK, "Switching to ASCII mode.");
    }
    else if (strcmp(sess->args, "I") == 0)
    {
        sess->ascii_mode = 0;
        ftp_reply(sess, FTP_TYPEOK, "Switching to Binary mode.");
    }
    else
    {
        ftp_reply(sess, FTP_BADCMD, "Unrecognised TYPE command.");
    }
}

void do_stru(session_t *sess)
{

}

void do_mode(session_t *sess)
{

}

void do_retr(session_t *sess)
{

}

void do_stor(session_t *sess)
{

}

void do_appe(session_t *sess)
{

}

void do_list(session_t *sess)
{
    int fd=tcp_client(0);
    int ret = connect_timeout(fd,sess->p_addr,tunable_connect_timeout);
    if(ret == -1)
        return ;
    sess->data_fd =fd;
    ftp_reply(sess,FTP_DATACONN,"Here comes the directory listing.");
    trans_list(sess);
    close(fd);
    sess->data_fd = -1;
    ftp_reply(sess,FTP_TRANSFEROK,"Directory send OK.");
}

void do_nlst(session_t *sess)
{

}

void do_rest(session_t *sess)
{

}

void do_abor(session_t *sess)
{

}

void do_pwd(session_t *sess)
{
    char tmp[1024] = {0};
    if(getcwd(tmp, sizeof tmp) == NULL)
    {
        fprintf(stderr, "get cwd error\n");
        ftp_reply(sess, FTP_BADMODE, "error");
        return;
    }
    char text[1024] = {0};
    snprintf(text, sizeof text, "\"%s\"", tmp);
    ftp_reply(sess, FTP_PWDOK, text);
}

void do_mkd(session_t *sess)
{

}

void do_rmd(session_t *sess)
{

}

void do_dele(session_t *sess)
{

}

void do_rnfr(session_t *sess)
{

}

void do_rnto(session_t *sess)
{

}

void do_site(session_t *sess)
{

}

void do_syst(session_t *sess)
{
    ftp_reply(sess, FTP_SYSTOK, "UNIX Type: L8");
}

void do_feat(session_t *sess)
{
    //211-Features:
    ftp_reply(sess, FTP_FEAT, "-Features:");

    //EPRT
    writen(sess->peerfd, " EPRT\r\n", strlen(" EPRT\r\n"));
    writen(sess->peerfd, " EPSV\r\n", strlen(" EPSV\r\n"));
    writen(sess->peerfd, " MDTM\r\n", strlen(" MDTM\r\n"));
    writen(sess->peerfd, " PASV\r\n", strlen(" PASV\r\n"));
    writen(sess->peerfd, " REST STREAM\r\n", strlen(" REST STREAM\r\n"));
    writen(sess->peerfd, " SIZE\r\n", strlen(" SIZE\r\n"));
    writen(sess->peerfd, " TVFS\r\n", strlen(" TVFS\r\n"));
    writen(sess->peerfd, " UTF8\r\n", strlen(" UTF8\r\n"));

    //211 End
    ftp_reply(sess, FTP_FEAT, "End");
}

void do_size(session_t *sess)
{

}

void do_stat(session_t *sess)
{

}

void do_noop(session_t *sess)
{

}

void do_help(session_t *sess)
{

}
