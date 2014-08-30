#include "ftp_proto.h"
#include "ftp_code.h"
#include "sysutil.h"
#include "strutil.h"
#include "command_map.h"
void clean_command(session_t *);
void handle_proto(session_t *sess)
{
          ftp_reply(sess,FTP_GREET,"FTP Server 1.0");
        while(1)
        {
          int ret = readline(sess->peerfd,sess->command,sizeof sess->command );
          if(ret==-1 )
          {
            if(errno == EINTR)
                continue;
            ERR_EXIT("readline");
          }
          else if(ret==0)
              exit(EXIT_SUCCESS);
          str_trim_crlf(sess->command);
          str_split(sess->command,sess->comm,sess->args,' ');
          printf("COMMD=[%s],ARGS=[%s]\n",sess->comm,sess->args);
           do_command_map(sess);
          clean_command(sess);
          }
}
void clean_command(session_t *sess)
{
            memset(sess->command,0x00,sizeof(sess->command));
            memset(sess->comm,0x00,sizeof(sess->comm));
            memset(sess->args,0x00,sizeof(sess->args));

}
