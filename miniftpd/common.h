#ifndef COMMON_H
#define COMMON_H 
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>
#include <sys/wait.h>
#include <crypt.h>
#include <shadow.h>
#include <dirent.h>
#include <time.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <linux/capability.h>
#include <sys/syscall.h>
#include <bits/syscall.h>
#include <sys/sendfile.h>

#define ERR_EXIT(m) \
    do { \
        perror(m);\
        exit(EXIT_FAILURE);\
    }while(0)


#endif  /*COMMON_H*/
