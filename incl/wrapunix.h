#ifndef __WRAP_UNIX_H
#define __WRAP_UNIX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <setjmp.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <atmi.h>

#define LOG "log/comm.log"
#define E_OK 0
#define E_FAIL -1

#define RESPCODE_OK "0"         /*0-通讯成功*/
#define RESPCODE_TIMEOUT "1"    /*1-超时*/
#define RESPCODE_TUXEDO_ERR "2" /*2-Tuxedo错误*/

#define CFG_FILENAME  "etc/comm_server.cfg"
#define TIMEOUT 60 /*超时时间*/
#define PKGHEAD_LENGTH 8 /*报文格式：8位长度PKGHEAD_LENGTH(不足时左补零)$Tuxedo交易码$数据包*/
#define MAXBUF_LEN 10240
#define MAX_LINE_LEGTH 4096
#define MAX_PK_LEN     4096

#define SS_BROKEN       0X01
#define SS_TIMEDOUT     0X02
#define SS_READABLE     0X03
#define SS_WRITEABLE    0X04

#define DEFAULTBUFLEN	65535
#define MINSOCKBUFLEN	255

#define SELECT_ARG1 int 
#define SELECT_ARG234 fd_set * 
#define SELECT_ARG5 struct timeval * 

#define min(a,b)            ((a)>(b) ? (b) : (a))
#endif

