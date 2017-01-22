#include  "comm_func.h"

jmp_buf  T_TmOutEnv;
struct comm_svr_socket s_comm_svr_socket;
 
 
/*

  内部系统----〉转发器--->自来水公司
  (1)内部系统与转发器
    通讯协议：socket 流式报文
    报文格式：响应码|Tuxedo交易码|数据包
  	     响应码:  
   	           0 通讯成功 
   	           1 超时 
   	           2 Tuxedo错误 
   	           3 报文超过4096
   	           9 默认
  (2)转发器与自来水公司
      通讯协议：Tuxedo String WSL、交易码、数据包	    
*/ 
 
 static void  func_timeout_handler( int rcvsig )
{
	if ( rcvsig == SIGALRM ){
			CommLog(LOG , "[%s][%d] 子进程 TIMEOUT DO LONGJMP  ",__FILE__,__LINE__ );
			longjmp( T_TmOutEnv, 1 );
	}
}	

/* 进程退出时信号处理 */
static void func_sigterm(int signo)
{
	CommLog(LOG , "[%s][%d]进程退出时信号处理 [%d]" ,__FILE__,__LINE__,signo); 
	exit( 0 );
}


/* 
    内部系统与转发器
    通讯协议：socket 流式报文
    报文格式：响应码$Tuxedo交易码$数据包
  	     响应码:  
   	           0 通讯成功 
   	           1 超时 
   	           2 Tuxedo错误 
   	           3 报文超过4096
   	           9 默认
 */

int process_parse(int conn)
{
	  int  ret ;
	  int  headlen = 0;
    int  msglen = 0;
    int  sendlen = 0;
    int  recvlen = 0;
   	char recv_head[128];
   	char recv_msg[MAXBUF_LEN];
   	char send_msg[MAXBUF_LEN];
   	char *p_str=NULL;
   	char tran_code[64];
   	char request_data[MAXBUF_LEN];
   	char response_data[MAXBUF_LEN];
   	char resp_code[16];
   	int count = 0;
   	int tmot  = 0;
   	
   	memset(resp_code, 0x00, sizeof(resp_code));
   	memset(recv_head, 0x00, sizeof(recv_head));
   	memset(recv_msg, 0x00, sizeof(recv_msg));
   	memset(send_msg, 0x00, sizeof(send_msg));
   	memset(tran_code, 0x00, sizeof(tran_code));
   	memset(request_data, 0x00, sizeof(request_data));
   	memset(response_data, 0x00, sizeof(response_data));


   	tmot=s_comm_svr_socket.water_timeout/10;
   	
   	CommLog(LOG , "[%s][%d] 转发器业务处理子进程：接收报文耗时[%d]",__FILE__,__LINE__ ,tmot);
   	recv_nbytes(conn,recv_msg,MAXBUF_LEN,tmot);
   	
   	CommLog(LOG , "[%s][%d] 转发器业务处理子进程：转发器接收内部系统报文[%s],len[%d]",__FILE__,__LINE__ ,recv_msg,strlen(recv_msg));
   	count = 1; /*响应码*/
   	
   	/*Tuxedo交易码*/
		count = 2;
		memset(tran_code, 0x00, sizeof(tran_code));
   	p_str=get_str(recv_msg,&count,"$");
   	strncpy(tran_code,p_str,sizeof(tran_code)-1);
    tran_code[sizeof(tran_code)-1] = '\0';
    
    /*数据包*/
   	count = 3;
		memset(request_data, 0x00, sizeof(request_data));
   	p_str=get_str(recv_msg,&count,"$");
   	strncpy(request_data,p_str,sizeof(request_data)-1);
    request_data[sizeof(request_data)-1] = '\0';
    
    
     
    bcl_ltrim(tran_code);
    bcl_rtrim(tran_code);
    
    CommLog(LOG , "[%s][%d] 转发器业务处理子进程：报文格式    响应码$Tuxedo交易码$数据包",__FILE__,__LINE__ );
    CommLog(LOG , "[%s][%d] 转发器业务处理子进程：响应码说明  0-通讯成功,1-超时,2-Tuxedo错误,9 默认",__FILE__,__LINE__ );
    CommLog(LOG , "[%s][%d] 转发器业务处理子进程：Tuxedo交易码          [%s] ",__FILE__,__LINE__ ,tran_code);
    CommLog(LOG , "[%s][%d] 转发器业务处理子进程：Tuxedo交易请求数据包  [%s] ",__FILE__,__LINE__ ,request_data);
    CommLog(LOG , "[%s][%d] 转发器业务处理子进程：Tuxedo交易环境变量    [%s] ",__FILE__,__LINE__ ,s_comm_svr_socket.tux_envstr);
   	
   	ret = comm_tux_tpcall(tran_code,request_data, response_data,s_comm_svr_socket.tux_envstr);
   	CommLog(LOG , "[%s][%d] Tuxedo交易响应数据包  [%s] ,长度[%d]",__FILE__,__LINE__ ,response_data,strlen(response_data));
   	if(ret != E_OK){
   	  	/*Tuxedo通讯失败*/
   	  	memset(resp_code, 0x00, sizeof(resp_code));
   	    strncpy(resp_code,RESPCODE_TUXEDO_ERR,sizeof(resp_code)-1);
        resp_code[sizeof(resp_code)-1]='\0';   	
   	}else{/*Tuxedo通讯成功*/
   		  memset(resp_code, 0x00, sizeof(resp_code));
   	    strncpy(resp_code,RESPCODE_OK,sizeof(resp_code)-1);
   	    resp_code[sizeof(resp_code)-1]='\0';   	
   	}
   	
   	/*报文格式：报文格式：响应码$Tuxedo交易码$数据包   */
   	memset(send_msg, 0x00, sizeof(send_msg));
   	snprintf(send_msg,sizeof(send_msg),"%s$%s$%s",resp_code,tran_code,response_data);
   	
   	sendlen = strlen(send_msg);
   	CommLog(LOG , "[%s][%d] 转发器业务处理子进程：响应码  [%s] ",__FILE__,__LINE__ ,resp_code);
   	CommLog(LOG , "[%s][%d] 转发器业务处理子进程：交易码  [%s] ",__FILE__,__LINE__ ,tran_code);
   	CommLog(LOG , "[%s][%d] 转发器业务处理子进程：响应报文[%s] ",__FILE__,__LINE__ ,send_msg);
   	CommLog(LOG , "[%s][%d] 转发器业务处理子进程：响应报文长度[%d] ",__FILE__,__LINE__,sendlen);
   	
   	
   	ret = send_nbytes(conn,send_msg,sendlen);
   	if(ret != sendlen){
   	  CommLog(LOG , "[%s][%d] 转发器业务处理子进程：返回响应报文失败[%d],socket[%d] ",__FILE__,__LINE__ ,ret,conn);
		  return E_FAIL;
   	}
   	
   	return E_OK;
	
}


 int process_main(int conn)
{
   	unsigned short  port_s ;
   	int      srv_port ;
   	int      timeout = TIMEOUT;
   	int      ret ;	
   	int      sendlen ;	
    char resp_code[16];
    char send_msg[MAXBUF_LEN];
    
   	
   	ret = get_local_port(conn,&port_s);
   	if(ret != E_OK){
		   CommLog(LOG , "[%s][%d] 子进程取端口号错  ",__FILE__,__LINE__ );
	   	 return -1 ;
	  }
	  srv_port = port_s;
	  CommLog(LOG , "[%s][%d] 子进程取端口号[%d]",__FILE__,__LINE__ ,srv_port);
   	/*设置超时*/
   	CommLog(LOG , "[%s][%d] 子进程设置超时[%d]",__FILE__,__LINE__ ,s_comm_svr_socket.water_timeout);
   	signal( SIGALRM, func_timeout_handler );
   	alarm( s_comm_svr_socket.water_timeout );
   	
   	if( setjmp( T_TmOutEnv ) != 0 ){/*超时*/
   		CommLog(LOG , "[%s][%d] 子进程  交易超时 [%d]",__FILE__,__LINE__ ,timeout);
   		/*返回超时响应包*/
   		memset(resp_code, 0x00, sizeof(resp_code)); 
   		memset(send_msg, 0x00, sizeof(send_msg));
   		
   		strncpy(resp_code,RESPCODE_TIMEOUT,sizeof(resp_code)-1);
   	  resp_code[sizeof(resp_code)-1]='\0';   
   	  
   		
   		/*报文格式：响应码|Tuxedo交易码|数据包*/
     	snprintf(send_msg,sizeof(send_msg),"%s$$",resp_code);
		  sendlen =strlen(send_msg);
		  
		  CommLog(LOG , "[%s][%d] 响应报文send_msg[%s] ",__FILE__,__LINE__ ,send_msg);
   	  CommLog(LOG , "[%s][%d] 响应报文长度[%d] ",__FILE__,__LINE__ ,sendlen);
   	  ret = send_nbytes(conn,send_msg,sendlen);
   	  if(ret != sendlen){
   	     CommLog(LOG , "[%s][%d] 返回响应报文失败[%d],socket[%d] ",__FILE__,__LINE__ ,ret,conn);
		     return E_FAIL;
   	  }
		  CommLog(LOG , "[%s][%d] 子进程  处理结束 ",__FILE__,__LINE__ );
		  return E_FAIL;
	  }
    CommLog(LOG , "[%s][%d] 子进程  接收数据 ",__FILE__,__LINE__ );
   	
   	ret = process_parse(conn);
   	if(ret != E_OK){
   		CommLog(LOG , "[%s][%d] 子进程 处理失败ret[%d] ",__FILE__,__LINE__ ,ret);
   		return E_FAIL;
   	}
   	return E_OK;
} 



int  fork_proc(int connect)
{
	pid_t   pid ;
	int     ret ;
	pid = fork();
	
	switch ( pid ) {
	  case -1: /*fork error*/
	  	return -1;
	  case 0:
	  	CommLog(LOG , "[%s][%d] 子进程 begin,pid[%d]  ",__FILE__,__LINE__, getpid() );
	  	CommLog(LOG , "[%s][%d] 子进程调用服务  ",__FILE__,__LINE__ );
	  	
	  	process_main(connect);
	  	ret = sock_close( connect );
	  	CommLog(LOG , "[%s][%d] 子进程关闭连接FD[%d]ret[%d]  ",__FILE__,__LINE__,connect,ret );
	  	CommLog(LOG , "[%s][%d] 子进程退出exit()  ",__FILE__,__LINE__ );
	    exit( 0 );
	  default: /*parent */
	  	return pid;		
   }
}    
void parent_proc(int listen_fd)
{
	  int max_listen_fd = listen_fd;
	  fd_set          rset ;
	  int rets = 0;
	  int ret = 0;
	  int n_connect = 0;
	  unsigned long   cli_addr ;     /*客户ip*/
   	unsigned short  cli_port ;
   	struct  timeval tval ;
	  
	  CommLog(LOG , "[%s][%d]parent_proc begin,listen_fd[%d]" ,__FILE__,__LINE__,listen_fd); 
	  CommLog(LOG , "[%s][%d] 父进程 select开始 监听描述符listen_fd[%d]" ,__FILE__,__LINE__,listen_fd);  
	  while(1){
	      FD_ZERO(&rset);
	      FD_SET(listen_fd,&rset);	
	      tval.tv_sec  = 0 ;
		    tval.tv_usec = 100000  ;
	      rets = select(max_listen_fd+1,&rset,NULL,NULL,&tval) ;
	      	 
	      if(rets < 0 ){/* 中断 */
		    	CommLog(LOG , "[%s][%d] 父进程 select 返回值[%d]errno[%d] continue" ,__FILE__,__LINE__,rets,errno, strerror(errno)); 
		    	continue ;
		    }else if( rets == 0 ){ /* 超时  */	
		    	/*CommLog(LOG , "[%s][%d] 父进程 select 超时" ,__FILE__,__LINE__); */
		    	continue ;
		    }else{/**/
		    	 CommLog(LOG , "[%s][%d] 父进程 select 结束,listen_fd[%d]" ,__FILE__,__LINE__,listen_fd); 
		    	 n_connect = tcp_accept (listen_fd, &cli_addr,&cli_port );
		    	 if( n_connect < 0 ){
			       CommLog(LOG , "[%s][%d] 父进程 accept connect is [%d] continue ",__FILE__,__LINE__, n_connect );
			       continue ;
		       }
		       CommLog(LOG , "[%s][%d] 父进程 accept connect is [%d]  ",__FILE__,__LINE__, n_connect );
           ret = fork_proc(n_connect);
           CommLog(LOG , "[%s][%d] 子进程 pid [%d],进行业务处理  ",__FILE__,__LINE__, ret );
		       ret = sock_close( n_connect );	
		       CommLog(LOG , "[%s][%d] 父进程 关闭 连接fd [%d]  ",__FILE__,__LINE__, n_connect );
		    }
		    CommLog(LOG , "[%s][%d] 父进程 select开始，监听描述符listen_fd[%d]" ,__FILE__,__LINE__,listen_fd);  
	}
	
}


/*服务主进程*/
void do_process(void)
{
  
  int		listen_fd = -1 ;/*监听描述符，*/
  int   ret = -1;
  char  str_value[128];
  char  cfg_path[256];
  
	signal( SIGHUP,  SIG_IGN );
	signal( SIGCLD,  SIG_IGN );
	signal( SIGCHLD ,SIG_IGN );	
	signal( SIGPIPE ,SIG_IGN );
	signal( SIGQUIT , func_sigterm );
	signal( SIGTERM , func_sigterm );
	
	memset(cfg_path,0x00,sizeof(cfg_path));
  snprintf(cfg_path,sizeof(cfg_path),"%s/%s",getenv("HOME"),CFG_FILENAME);	
	
	CommLog(LOG , "[%s][%d]do_process begin" ,__FILE__,__LINE__); 
	/*获取配置信息*/
	memset(&s_comm_svr_socket, 0x00, sizeof(struct comm_svr_socket));
	ret = get_comm_svr_cfg(cfg_path,&s_comm_svr_socket);
	if(ret != E_OK ){
	  CommLog(LOG , "[%s][%d] 子进程退出exit" ,__FILE__,__LINE__); 	
	 	exit(-1);
	}
	snprintf (s_comm_svr_socket.tux_envstr,sizeof(s_comm_svr_socket.tux_envstr),"WSNADDR=//%s:%s",s_comm_svr_socket.water_ip,s_comm_svr_socket.water_port);
	bcl_trimall(s_comm_svr_socket.tux_envstr);
	
	CommLog(LOG , "[%s][%d]  转发器监听端口  [%d]" ,__FILE__,__LINE__,s_comm_svr_socket.comm_svr_port); 
	CommLog(LOG , "[%s][%d]  自来水公司IP    [%s]" ,__FILE__,__LINE__,s_comm_svr_socket.water_ip); 
	CommLog(LOG , "[%s][%d]  自来水公司端口  [%s]" ,__FILE__,__LINE__,s_comm_svr_socket.water_port); 
	CommLog(LOG , "[%s][%d]  tux_envstr      [%s]" ,__FILE__,__LINE__,s_comm_svr_socket.tux_envstr); 
	CommLog(LOG , "[%s][%d]  超时时间        [%d]" ,__FILE__,__LINE__,s_comm_svr_socket.water_timeout); 
	
	ret = tcp_listener( &listen_fd, htonl( INADDR_ANY ), s_comm_svr_socket.comm_svr_port );
	if(ret != E_OK){
		CommLog(LOG , "[%s][%d]tcp_listener error ,s_comm_svr_socket.comm_svr_port[%d]" ,__FILE__,__LINE__,s_comm_svr_socket.comm_svr_port); 
		exit(-1);
	}
		
	parent_proc(listen_fd);	/*处理异常，子进程成功、失败后都exit*/
	
}



int main(int argc,char **argv)
{
	
	pid_t    pid = -1;
	pid_t   wpid = -1;
	pid_t   cpid = -1;
	int     stat = 0;
	
	 /*守护进程*/
	 /* 转成监控进程 */
	run_daemon(getenv("HOME"));
	
	signal( SIGCLD,  SIG_IGN );	
	signal( SIGCHLD ,SIG_DFL );	
	
   /* 启动主进程，执行业务逻辑*/
   pid=fork();
   if(pid <0 ){ 
   	fprintf(stderr,"fork() error\n");
		fflush(stderr);
		exit(-1);
   }else if( pid == 0 ){
	   	do_process();
   }
    
    /*监视主进程,进程退出后，重新fork子进程cpid*/
    while(1){
       while ( ( wpid = waitpid(-1, &stat, WNOHANG)) > 0 ) {
       	CommLog(LOG , "[%s][%d] 监视主进程：子进程退出exit后，重新fork子进程" ,__FILE__,__LINE__); 	
       	
		    cpid=fork();
		    if (cpid<0){
		   	 	 printf("keepalive can not fork new process\n");
		   		 exit(-1);
	      }else if (cpid==0){     
		   		do_process();
		   	}			
		   }
		   
		   sleep(5);	
    }
    
	 return 0;
}

int process_main_20160927(int conn)
{
   	unsigned short  port_s ;
   	int      srv_port ;
   	int      timeout = TIMEOUT;
   	int      ret ;	
   	int      sendlen ;	
    char resp_code[16];
    char send_msg[MAXBUF_LEN];
    
   	
   	ret = get_local_port(conn,&port_s);
   	if(ret != E_OK){
		   CommLog(LOG , "[%s][%d] 子进程取端口号错  ",__FILE__,__LINE__ );
	   	 return -1 ;
	  }
	  srv_port = port_s;
	  CommLog(LOG , "[%s][%d] 子进程取端口号[%d]",__FILE__,__LINE__ ,srv_port);
   	/*设置超时*/
   	CommLog(LOG , "[%s][%d] 子进程设置超时[%d]",__FILE__,__LINE__ ,s_comm_svr_socket.water_timeout);
   	signal( SIGALRM, func_timeout_handler );
   	alarm( s_comm_svr_socket.water_timeout );
   	
   	if( setjmp( T_TmOutEnv ) != 0 ){/*超时*/
   		CommLog(LOG , "[%s][%d] 子进程  交易超时 [%d]",__FILE__,__LINE__ ,timeout);
   		/*返回超时响应包*/
   		memset(resp_code, 0x00, sizeof(resp_code)); 
   		memset(send_msg, 0x00, sizeof(send_msg));
   		
   		strncpy(resp_code,RESPCODE_TIMEOUT,sizeof(resp_code)-1);
   	  resp_code[sizeof(resp_code)-1]='\0';   
   	  
   		sendlen =PKGHEAD_LENGTH + 4 +strlen(resp_code);
     	snprintf(send_msg,sizeof(send_msg),"%08d$%s$$$",sendlen-PKGHEAD_LENGTH,resp_code);
		  
		  CommLog(LOG , "[%s][%d] 响应报文send_msg[%s] ",__FILE__,__LINE__ ,send_msg);
   	  CommLog(LOG , "[%s][%d] 响应报文长度[%d] ",__FILE__,__LINE__ ,sendlen);
   	  ret = send_nbytes(conn,send_msg,sendlen);
   	  if(ret != sendlen){
   	     CommLog(LOG , "[%s][%d] 返回响应报文失败[%d],socket[%d] ",__FILE__,__LINE__ ,ret,conn);
		     return E_FAIL;
   	  }
		  CommLog(LOG , "[%s][%d] 子进程  处理结束 ",__FILE__,__LINE__ );
		  return E_FAIL;
	  }
    CommLog(LOG , "[%s][%d] 子进程  接收数据 ",__FILE__,__LINE__ );
   	
   	ret = process_parse(conn);
   	if(ret != E_OK){
   		CommLog(LOG , "[%s][%d] 子进程 处理失败ret[%d] ",__FILE__,__LINE__ ,ret);
   		return E_FAIL;
   	}
   	return E_OK;
} 

/*报文格式：8位长度$响应码$Tuxedo交易码$数据包$
   	    包头：  8位长度,不足时左补零
   	    包体：  $响应码$Tuxedo交易码$数据包$
   	    长度:   包体长度
   	    响应码: 0 通讯成功 1超时 2 tuxedo通讯失败 默认填9
 */

int process_parse_20160927(int conn)
{
	  int  ret ;
	  int  headlen = 0;
    int  msglen = 0;
    int  sendlen = 0;
    int  recvlen = 0;
   	char recv_head[128];
   	char recv_msg[MAXBUF_LEN];
   	char send_msg[MAXBUF_LEN];
   	char *p_str=NULL;
   	char tran_code[64];
   	char request_data[MAXBUF_LEN];
   	char response_data[MAXBUF_LEN];
   	char resp_code[16];
   	int count = 0;
   	
   	memset(resp_code, 0x00, sizeof(resp_code));
   	memset(recv_head, 0x00, sizeof(recv_head));
   	memset(recv_msg, 0x00, sizeof(recv_msg));
   	memset(send_msg, 0x00, sizeof(send_msg));
   	memset(tran_code, 0x00, sizeof(tran_code));
   	memset(request_data, 0x00, sizeof(request_data));
   	memset(response_data, 0x00, sizeof(response_data));

   	headlen = recvn(conn,recv_head,PKGHEAD_LENGTH);
   	if(headlen != PKGHEAD_LENGTH){
   	  CommLog(LOG , "[%s][%d] 子进程 ,报文头不足8位recv_head [%s]",__FILE__,__LINE__ ,recv_head);
		  return -1;
   	}
   	
   	msglen = atoi(recv_head);
   	
   	CommLog(LOG , "[%s][%d] 子进程 ,报文格式：8位长度$Tuxedo交易码$数据包$，包头长度只包括包体",__FILE__,__LINE__ );
   	CommLog(LOG , "[%s][%d] 子进程 ,8位报文头recv_head [%s],msglen[%d]",__FILE__,__LINE__ ,recv_head,msglen);
   	memset(recv_msg, 0x00, sizeof(recv_msg));
   	memcpy(recv_msg,recv_head,PKGHEAD_LENGTH);
   	ret = recvn(conn,recv_msg+PKGHEAD_LENGTH,msglen);
   	if(ret != msglen){
   		CommLog(LOG , "[%s][%d] 子进程 ,报文体格式错,长度不足[%d],实际[%d],recv_msg[%s] ",__FILE__,__LINE__ ,msglen,ret,recv_msg);
		  return -1;
   	}
   	recvlen = PKGHEAD_LENGTH + msglen;
   	CommLog(LOG , "[%s][%d] 报文recv_msg[%s] ",__FILE__,__LINE__ ,recv_msg);
   	CommLog(LOG , "[%s][%d] 报文长度[%d] ",__FILE__,__LINE__ ,recvlen);
   	
   	memset(tran_code, 0x00, sizeof(tran_code));
   	count = 3;
   	p_str=get_str(recv_msg,&count,"$");
   	strncpy(tran_code,p_str,sizeof(tran_code)-1);
    tran_code[sizeof(tran_code)-1] = '\0';
    
    memset(request_data, 0x00, sizeof(request_data));
    count = 4;
   	p_str=get_str(recv_msg,&count,"$");
   	strncpy(request_data,p_str,sizeof(request_data)-1);
    request_data[sizeof(request_data)-1] = '\0';
    
    CommLog(LOG , "[%s][%d] Tuxedo交易码          [%s] ",__FILE__,__LINE__ ,tran_code);
    CommLog(LOG , "[%s][%d] Tuxedo交易请求数据包  [%s] ",__FILE__,__LINE__ ,request_data);
    CommLog(LOG , "[%s][%d] Tuxedo交易环境变量    [%s] ",__FILE__,__LINE__ ,s_comm_svr_socket.tux_envstr);
   	
   	ret = comm_tux_tpcall(tran_code,request_data, response_data,s_comm_svr_socket.tux_envstr);
   	CommLog(LOG , "[%s][%d] Tuxedo交易响应数据包  [%s] ,长度[%d]",__FILE__,__LINE__ ,response_data,strlen(response_data));
   	if(ret != E_OK){
   	  	/*Tuxedo通讯失败*/
   	  	memset(resp_code, 0x00, sizeof(resp_code));
   	    strncpy(resp_code,RESPCODE_TUXEDO_ERR,sizeof(resp_code)-1);
        resp_code[sizeof(resp_code)-1]='\0';   	
   	}else{/*Tuxedo通讯成功*/
   		  memset(resp_code, 0x00, sizeof(resp_code));
   	    strncpy(resp_code,RESPCODE_OK,sizeof(resp_code)-1);
   	    resp_code[sizeof(resp_code)-1]='\0';   	
   	}
   	
   	/*报文格式：8位长度$响应码$Tuxedo交易码$数据包$*/
   	sendlen =PKGHEAD_LENGTH + 4 +strlen(resp_code) +strlen(tran_code) +strlen(response_data);
   	memset(send_msg, 0x00, sizeof(send_msg));
   	snprintf(send_msg,sizeof(send_msg),"%08d$%s$%s$%s$",sendlen-PKGHEAD_LENGTH,resp_code,tran_code,response_data);
   	
   	CommLog(LOG , "[%s][%d] 响应报文send_msg[%s] ",__FILE__,__LINE__ ,send_msg);
   	CommLog(LOG , "[%s][%d] 响应报文长度[%d] ",__FILE__,__LINE__ ,sendlen);
   	ret = send_nbytes(conn,send_msg,sendlen);
   	if(ret != sendlen){
   	  CommLog(LOG , "[%s][%d] 返回响应报文失败[%d],socket[%d] ",__FILE__,__LINE__ ,ret,conn);
		  return E_FAIL;
   	}
   	
   	return E_OK;
	
}
