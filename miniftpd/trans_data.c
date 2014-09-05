#include "trans_data.h"
#include "ftp_code.h"
#include "command_map.h"
#include "common.h"
#include "sysutil.h"
#include "configure.h"
#include "priv_sock.h"
#include "priv_command.h"

static const char *statbuf_get_perms(struct stat *sbuf);
static const char *statbuf_get_date(struct stat *sbuf);
static const char *statbuf_get_filename(struct stat *sbuf, const char *name);
static const char *statbuf_get_user_info(struct stat *sbuf);
static const char *statbuf_get_size(struct stat *sbuf);

static int is_port_active(session_t *sess);
static int is_pasv_active(session_t *sess);

static void get_port_data_fd(session_t *sess);
static void get_pasv_data_fd(session_t *sess);


//返回值表示成功与否,主动被动模式下的fd
int get_trans_data_fd(session_t *sess)
{
    int is_port = is_port_active(sess);
    int is_pasv = is_pasv_active(sess);
    if(!is_port && !is_pasv)
    {
          ftp_reply(sess, FTP_BADSENDCONN, "Use PORT or PASV first.");
          exit(EXIT_FAILURE);
    }

    if(is_port && is_pasv)
    {
        fprintf(stderr, "both of PORT and PASV are active\n");
                exit(EXIT_FAILURE);
    }

    if(is_port)
    {
        get_port_data_fd(sess);
    }
    if(is_pasv)
    {
        get_pasv_data_fd(sess);
    }
    return 1;
}

void trans_list(session_t *sess)
{
    DIR *dir=opendir(".");
    if(dir == NULL)
        ERR_EXIT("opendir");
    struct dirent *entry;
    while((entry = readdir(dir)))
    {
        if(entry ->d_name[0]=='.')
            continue;
        const char *filename=entry->d_name;
        if(filename[0]=='.')
            continue;

        char buf[1024]={0};
        struct stat sbuf;

        if(lstat(filename,&sbuf)==-1)
            ERR_EXIT("lstat");
        strcpy(buf, statbuf_get_perms(&sbuf));
        strcat(buf, " ");
        strcat(buf, statbuf_get_user_info(&sbuf));
        strcat(buf, " ");
        strcat(buf, statbuf_get_size(&sbuf));
        strcat(buf, " ");
        strcat(buf, statbuf_get_date(&sbuf));
        strcat(buf, " ");
        strcat(buf, statbuf_get_filename(&sbuf, filename));

        strcat(buf, "\r\n");
        writen(sess->data_fd, buf, strlen(buf));
    }
    closedir(dir);
}
void trans_list_simple(session_t *sess)
{
    DIR *dir=opendir(".");
    if(dir == NULL)
        ERR_EXIT("opendir");
    struct dirent *entry;
    while((entry = readdir(dir)))
    {
        if(entry ->d_name[0]=='.')
            continue;
        const char *filename=entry->d_name;
        if(filename[0]=='.')
            continue;

        char buf[1024]={0};
        struct stat sbuf;

        if(lstat(filename,&sbuf)==-1)
            ERR_EXIT("lstat");
        strcpy(buf, statbuf_get_filename(&sbuf, filename));

        strcat(buf, "\r\n");
        writen(sess->data_fd, buf, strlen(buf));
    }
    closedir(dir);

}
static const char *statbuf_get_perms(struct stat *sbuf)
{
        //获取文件类型信息
        static char perms[]="----------";
        mode_t mode = sbuf->st_mode;
        switch(mode & S_IFMT)
        {
            case S_IFSOCK:
                perms[0]='s';
                break;
            case S_IFLNK:
                perms[0]='l';
                break;
            case S_IFREG:
                perms[0]='-';
                break;
            case S_IFBLK:
                perms[0]='b';
                break;
            case S_IFDIR:
                perms[0]='d';
                break;
            case S_IFCHR:
                perms[0]='c';
                break;
            case S_IFIFO:
                perms[0]='p';
                break;
        }
        //获取权限信息用户权限，用户所属组权限，其他人权限other
    if(mode & S_IRUSR)
        perms[1] = 'r';
    if(mode & S_IWUSR)
        perms[2] = 'w';
    if(mode & S_IXUSR)
        perms[3] = 'x';
    if(mode & S_IRGRP)
        perms[4] = 'r';
    if(mode & S_IWGRP)
        perms[5] = 'w';
    if(mode & S_IXGRP)
        perms[6] = 'x';
    if(mode & S_IROTH)
        perms[7] = 'r';
    if(mode & S_IWOTH)
        perms[8] = 'w';
    if(mode & S_IXOTH)
        perms[9] = 'x';

    if(mode & S_ISUID)
        perms[3]=(perms[3]=='x')?'s':'S';
    if(mode & S_ISGID)
        perms[6]=(perms[6] == 'x')?'s':'S';
    if(mode & S_ISVTX)
        perms[9]=(perms[9]=='x')?'t':'T';
    
    return perms;

}
static const char *statbuf_get_date(struct stat *sbuf)
{

    //获取时间
    static char datebuf[1024];
    struct tm *ptm;
    time_t ct =sbuf->st_ctime;

    if((ptm = localtime(&ct))==NULL)
        ERR_EXIT("localtime");

    const char *format="%b %e %H:%M";
    if(strftime(datebuf,sizeof(datebuf),format,ptm)==0)
    {
        fprintf(stderr,"strtime error\n");
        exit(EXIT_FAILURE);
    }
    return datebuf;

}
static const char *statbuf_get_filename(struct stat *sbuf, const char *name)
{
    static char filename[1024] = {0};
    //name 澶..?炬.?..
    if(S_ISLNK(sbuf->st_mode))
    {
        char linkfile[1024] = {0};
        if(readlink(name, linkfile, sizeof linkfile) == -1)
            ERR_EXIT("readlink");
        snprintf(filename, sizeof filename, " %s -> %s", name, linkfile);
    }else
    {
        strcpy(filename, name);
    }

    return filename; 

}
static const char *statbuf_get_user_info(struct stat *sbuf)
{
    //获取文件连接数拥有者信息拥有者所属组    
    static char info[1024]={0};
    snprintf(info,sizeof(info),"%3d %8d %8d",sbuf->st_nlink,sbuf->st_uid,sbuf->st_gid);
    return info;
}
static const char *statbuf_get_size(struct stat *sbuf)
{
    //获取文件大小
    static char sizebuf[100]={0};
    snprintf(sizebuf,sizeof(sizebuf),"%8lu",(unsigned long) sbuf->st_size);
    return sizebuf;

}

static int is_port_active(session_t *sess)
{
    return (sess->p_addr!=NULL);
}

static int is_pasv_active(session_t *sess)
{
    priv_sock_send_cmd(sess->proto_fd,PRIV_SOCK_PASV_ACTIVE);
    return priv_sock_recv_int(sess->proto_fd);
}

static void get_port_data_fd(session_t *sess)
{
        priv_sock_send_cmd(sess->proto_fd,PRIV_SOCK_GET_DATA_SOCK);
        char *ip = inet_ntoa(sess->p_addr->sin_addr);
        uint16_t port = ntohs(sess->p_addr->sin_port);
        priv_sock_send_str(sess->proto_fd,ip,strlen(ip));
        priv_sock_send_int(sess->proto_fd,port);

        //recv result
        char result = priv_sock_recv_result(sess->proto_fd);
        if(result == PRIV_SOCK_RESULT_BAD)
        {
            fprintf(stderr,"get data fd err");
            exit(EXIT_FAILURE);
        }
        sess->data_fd=priv_sock_recv_fd(sess->proto_fd);

        free(sess->p_addr);
        sess->p_addr=NULL;

}
static void get_pasv_data_fd(session_t *sess)
{
        priv_sock_send_cmd(sess->proto_fd,PRIV_SOCK_PASV_ACCEPT);
        char res=priv_sock_recv_result(sess->proto_fd);
        
        if(res==PRIV_SOCK_RESULT_BAD)
        {
             ftp_reply(sess, FTP_BADCMD, "get pasv data_fd error");
            fprintf(stderr,"get data fd error");
            exit(EXIT_FAILURE);
        }
        sess->data_fd = priv_sock_recv_fd(sess->proto_fd);
        //清除PASV模式
        close(sess->listen_fd);
        sess->listen_fd=-1;

}
/*
void upload_file(session_t *sess,int is_appe)
{
    //获取fd
    if(get_trans_data_fd(sess) ==0)
    {
        ftp_reply(sess,FTP_UPLOADFAIL,"Failed to get data fd.");
        return;
    }
    //open file
    int fd= open(sess->args,O_WRONLY|O_CREAT,0755);
    if(fd == -1)
    {
        ftp_reply(sess,FTP_UPLOADFAIL, "Failed to open file.");
        return;
    }
    //lock file
    if(lock_file_write(fd)==-1)
    {
     ftp_reply(sess, FTP_UPLOADFAIL, "Failed to lock file.");
     return;
    }
    //judge regular file
    struct stat sbuf;
    if(fstat(fd,&sbuf) ==-1)
    {
        ERR_EXIT("lstat");
    }
    if(!S_ISREG(sbuf.st_mode))
    {
        ftp_reply(sess,FTP_UPLOADFAIL,"Can only upload regular file.");
        return;
    }
    //breakpoint 
    uint64_t offset= sess->restart_pos;
    unsigned long filesize = 0;
    //区分模式 APPE or REST+STOR
    if(!is_appe && offset ==0)  //normally STOR
    {
        //截断函数
        ftruncate(fd,0); //如果源文件存在则直接覆盖
    }
    else if(!is_appe && offset != 0) //rest+stor
    {
        ftruncate(fd,offset);
        if(lseek(fd,offset,SEEK_SET)==-1)
            ERR_EXIT("lseek");
        filesize= offset;
    }
    else
    {
        //对文件进行扩展 偏移到末尾进行追加
        if(lseek(fd,0,SEEK_END) == -1)
            ERR_EXIT("lseek");
        if(fstat(fd,&sbuf) == -1)
            ERR_EXIT("fstat");
        filesize = sbuf.st_size;
    }
    //ascii
    char text[1024] = {0};
     if(sess->ascii_mode == 1)
         snprintf(text, sizeof text, "Opening ASCII mode data connection for %s (%lu bytes).", sess->args, filesize);
    else
          snprintf(text, sizeof text, "Opening Binary mode data connection for %s (%lu bytes).", sess->args, filesize);
    printf("11111\n");
     ftp_reply(sess, FTP_DATACONN, text);
    
    //upload

    char buf[4096]={0};
    int flag =0;
    while(1)
    {
        int ret = read(sess->data_fd,buf,sizeof buf);
        if(ret == -1)
        {
            if(errno == EINTR)
                continue;
            flag =1;
            break;
        }
        else if(ret ==0)
        {
            flag =0;
            break;
        }
        if(writen(fd,buf,ret) !=ret)
        {
            flag =2;
            break;
        }
         
    }
    //unlock file
    if(unlock_file(fd)==-1)
    {
        fprintf(stderr,"unlock error");
        return;
    }
    //close
    close(fd);
    close(sess->data_fd);
    sess->data_fd =-1;

    if(flag ==0)
        ftp_reply(sess,FTP_TRANSFEROK,"Transfer complete.");
    if(flag ==1)
        ftp_reply(sess,FTP_BADSENDNET,"Reading from Network error.");
    if(flag == 2)
        ftp_reply(sess,FTP_BADSENDFILE,"Writing to File Failed.");
}
*/

void upload_file(session_t *sess, int is_appe)
{
    //获取data fd
    if(get_trans_data_fd(sess) == 0)
    {
        ftp_reply(sess, FTP_UPLOADFAIL, "Failed to get data fd."); 
        return;
    }
        
    //open 文件
    int fd = open(sess->args, O_WRONLY | O_CREAT, 0666);
    if(fd == -1)
    {
        ftp_reply(sess, FTP_UPLOADFAIL, "Failed to open file."); 
        return;
    }

    //对文件加锁
    if(lock_file_write(fd) == -1)
    {
        ftp_reply(sess, FTP_UPLOADFAIL, "Failed to lock file."); 
        return;
    }

    //判断是否是普通文件
    struct stat sbuf;
    if(fstat(fd, &sbuf) == -1)
        ERR_EXIT("fstat");
    if(!S_ISREG(sbuf.st_mode))
    {
        ftp_reply(sess, FTP_UPLOADFAIL, "Can only upload regular file."); 
        return;
    }

    //区分模式
    long long offset = sess->restart_pos;
    unsigned long filesize = 0;
    if(!is_appe && offset == 0) //STOR
    {
        //创建新的文件
        ftruncate(fd, 0);   //如果源文件存在则直接覆盖
    }
    else if(!is_appe && offset != 0) // REST + STOR
    {
        //lseek进行偏移
        ftruncate(fd, offset);  //截断后面的内容
        if(lseek(fd, offset, SEEK_SET) == -1)
            ERR_EXIT("lseek");
        filesize = offset;
    }
    else    //APPE
    {
        //对文件进行扩展 偏移到末尾进行追加
        if(lseek(fd, 0, SEEK_END) == -1)
            ERR_EXIT("lseek");

        //获取文件大小
        if(fstat(fd, &sbuf) == -1)
            ERR_EXIT("fstat");
        filesize = sbuf.st_size;
    }

    //150 ascii
    //150 Opening ASCII mode data connection for /home/wing/redis-stable.tar.gz (1251318 bytes).
    char text[1024] = {0};
    if(sess->ascii_mode == 1)
        snprintf(text, sizeof text, "Opening ASCII mode data connection for %s (%lu bytes).", sess->args, filesize);
    else
        snprintf(text, sizeof text, "Opening Binary mode data connection for %s (%lu bytes).", sess->args, filesize);
    ftp_reply(sess, FTP_DATACONN, text);

    //上传
    char buf[4096] = {0};
    int flag = 0;
    while(1)
    {
        int nread = read(sess->data_fd, buf, sizeof buf);
        if(nread == -1)
        {
            if(errno == EINTR)
                continue;
            flag = 1;
            break;
        }
        else if(nread == 0)
        {
            flag = 0;
            break;
        }

        if(writen(fd, buf, nread) != nread)
        {
            flag = 2;
            break;
        }
    }

    //清理 关闭fd 文件解锁
    if(unlock_file(fd) == -1)
        ERR_EXIT("unlock_file");
    close(fd);
    close(sess->data_fd);
    sess->data_fd = -1;


    //226
    if(flag == 0)
        ftp_reply(sess, FTP_TRANSFEROK, "Transfer complete.");
    else if(flag == 1)
        ftp_reply(sess, FTP_BADSENDNET, "Reading from Network Failed.");
    else
        ftp_reply(sess, FTP_BADSENDFILE, "Writing to File Failed.");

}

