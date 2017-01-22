#ifndef __COMM_FUNC_H
#define __COMM_FUNC_H

#include "wrapunix.h"


struct comm_svr_socket{
  unsigned short comm_svr_port; /* 转发器监听端口 */
  char water_ip[64];      /*自来水公司IP */
  char water_port[8];    /*自来水公司端口*/
  int  water_timeout; /*转发器与自来水公司超时时间*/
  char tux_envstr[64];/*tuxedo WSNADDR*/
};

typedef	void	Sigfunc(int);	/* for signal handlers */

#ifdef __cplusplus
extern "C" {
#endif
  
 extern int CommLog(char *logname , char *strfmt,...);/*日志文件每天一个*/
 extern  char *bcl_trimall( char *str );
 extern  char *bcl_ltrim( char *str );
 extern  char *bcl_rtrim( char *str );
 extern  char *get_str(char *source,int *m,char *dlmt);

 extern int get_cfg_value( char* filepath ,  char *name , char *value );
 extern int get_comm_svr_cfg(char * cfg_path,struct comm_svr_socket *p);
 
 extern int comm_tux_tpcall(char*servicename, char *request_buf,char * response_buf,char *envstr);

 extern struct comm_svr_socket s_comm_svr_socket;
#ifdef __cplusplus
}
#endif

#endif
