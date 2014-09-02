#include "trans_data.h"
#include "common.h"
#include "sysutil.h"

static const char *statbuf_get_perms(struct stat *sbuf);
static const char *statbuf_get_date(struct stat *sbuf);
static const char *statbuf_get_filename(struct stat *sbuf, const char *name);
static const char *statbuf_get_user_info(struct stat *sbuf);
static const char *statbuf_get_size(struct stat *sbuf);

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
