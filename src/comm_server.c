#include  "comm_func.h"

jmp_buf  T_TmOutEnv;
struct comm_svr_socket s_comm_svr_socket;
 
 
/*

  �ڲ�ϵͳ----��ת����--->����ˮ��˾
  (1)�ڲ�ϵͳ��ת����
    ͨѶЭ�飺socket ��ʽ����
    ���ĸ�ʽ����Ӧ��|Tuxedo������|���ݰ�
  	     ��Ӧ��:  
   	           0 ͨѶ�ɹ� 
   	           1 ��ʱ 
   	           2 Tuxedo���� 
   	           3 ���ĳ���4096
   	           9 Ĭ��
  (2)ת����������ˮ��˾
      ͨѶЭ�飺Tuxedo String WSL�������롢���ݰ�	    
*/ 
 
 static void  func_timeout_handler( int rcvsig )
{
	if ( rcvsig == SIGALRM ){
			CommLog(LOG , "[%s][%d] �ӽ��� TIMEOUT DO LONGJMP  ",__FILE__,__LINE__ );
			longjmp( T_TmOutEnv, 1 );
	}
}	

/* �����˳�ʱ�źŴ��� */
static void func_sigterm(int signo)
{
	CommLog(LOG , "[%s][%d]�����˳�ʱ�źŴ��� [%d]" ,__FILE__,__LINE__,signo); 
	exit( 0 );
}


/* 
    �ڲ�ϵͳ��ת����
    ͨѶЭ�飺socket ��ʽ����
    ���ĸ�ʽ����Ӧ��$Tuxedo������$���ݰ�
  	     ��Ӧ��:  
   	           0 ͨѶ�ɹ� 
   	           1 ��ʱ 
   	           2 Tuxedo���� 
   	           3 ���ĳ���4096
   	           9 Ĭ��
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
   	
   	CommLog(LOG , "[%s][%d] ת����ҵ�����ӽ��̣����ձ��ĺ�ʱ[%d]",__FILE__,__LINE__ ,tmot);
   	recv_nbytes(conn,recv_msg,MAXBUF_LEN,tmot);
   	
   	CommLog(LOG , "[%s][%d] ת����ҵ�����ӽ��̣�ת���������ڲ�ϵͳ����[%s],len[%d]",__FILE__,__LINE__ ,recv_msg,strlen(recv_msg));
   	count = 1; /*��Ӧ��*/
   	
   	/*Tuxedo������*/
		count = 2;
		memset(tran_code, 0x00, sizeof(tran_code));
   	p_str=get_str(recv_msg,&count,"$");
   	strncpy(tran_code,p_str,sizeof(tran_code)-1);
    tran_code[sizeof(tran_code)-1] = '\0';
    
    /*���ݰ�*/
   	count = 3;
		memset(request_data, 0x00, sizeof(request_data));
   	p_str=get_str(recv_msg,&count,"$");
   	strncpy(request_data,p_str,sizeof(request_data)-1);
    request_data[sizeof(request_data)-1] = '\0';
    
    
     
    bcl_ltrim(tran_code);
    bcl_rtrim(tran_code);
    
    CommLog(LOG , "[%s][%d] ת����ҵ�����ӽ��̣����ĸ�ʽ    ��Ӧ��$Tuxedo������$���ݰ�",__FILE__,__LINE__ );
    CommLog(LOG , "[%s][%d] ת����ҵ�����ӽ��̣���Ӧ��˵��  0-ͨѶ�ɹ�,1-��ʱ,2-Tuxedo����,9 Ĭ��",__FILE__,__LINE__ );
    CommLog(LOG , "[%s][%d] ת����ҵ�����ӽ��̣�Tuxedo������          [%s] ",__FILE__,__LINE__ ,tran_code);
    CommLog(LOG , "[%s][%d] ת����ҵ�����ӽ��̣�Tuxedo�����������ݰ�  [%s] ",__FILE__,__LINE__ ,request_data);
    CommLog(LOG , "[%s][%d] ת����ҵ�����ӽ��̣�Tuxedo���׻�������    [%s] ",__FILE__,__LINE__ ,s_comm_svr_socket.tux_envstr);
   	
   	ret = comm_tux_tpcall(tran_code,request_data, response_data,s_comm_svr_socket.tux_envstr);
   	CommLog(LOG , "[%s][%d] Tuxedo������Ӧ���ݰ�  [%s] ,����[%d]",__FILE__,__LINE__ ,response_data,strlen(response_data));
   	if(ret != E_OK){
   	  	/*TuxedoͨѶʧ��*/
   	  	memset(resp_code, 0x00, sizeof(resp_code));
   	    strncpy(resp_code,RESPCODE_TUXEDO_ERR,sizeof(resp_code)-1);
        resp_code[sizeof(resp_code)-1]='\0';   	
   	}else{/*TuxedoͨѶ�ɹ�*/
   		  memset(resp_code, 0x00, sizeof(resp_code));
   	    strncpy(resp_code,RESPCODE_OK,sizeof(resp_code)-1);
   	    resp_code[sizeof(resp_code)-1]='\0';   	
   	}
   	
   	/*���ĸ�ʽ�����ĸ�ʽ����Ӧ��$Tuxedo������$���ݰ�   */
   	memset(send_msg, 0x00, sizeof(send_msg));
   	snprintf(send_msg,sizeof(send_msg),"%s$%s$%s",resp_code,tran_code,response_data);
   	
   	sendlen = strlen(send_msg);
   	CommLog(LOG , "[%s][%d] ת����ҵ�����ӽ��̣���Ӧ��  [%s] ",__FILE__,__LINE__ ,resp_code);
   	CommLog(LOG , "[%s][%d] ת����ҵ�����ӽ��̣�������  [%s] ",__FILE__,__LINE__ ,tran_code);
   	CommLog(LOG , "[%s][%d] ת����ҵ�����ӽ��̣���Ӧ����[%s] ",__FILE__,__LINE__ ,send_msg);
   	CommLog(LOG , "[%s][%d] ת����ҵ�����ӽ��̣���Ӧ���ĳ���[%d] ",__FILE__,__LINE__,sendlen);
   	
   	
   	ret = send_nbytes(conn,send_msg,sendlen);
   	if(ret != sendlen){
   	  CommLog(LOG , "[%s][%d] ת����ҵ�����ӽ��̣�������Ӧ����ʧ��[%d],socket[%d] ",__FILE__,__LINE__ ,ret,conn);
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
		   CommLog(LOG , "[%s][%d] �ӽ���ȡ�˿ںŴ�  ",__FILE__,__LINE__ );
	   	 return -1 ;
	  }
	  srv_port = port_s;
	  CommLog(LOG , "[%s][%d] �ӽ���ȡ�˿ں�[%d]",__FILE__,__LINE__ ,srv_port);
   	/*���ó�ʱ*/
   	CommLog(LOG , "[%s][%d] �ӽ������ó�ʱ[%d]",__FILE__,__LINE__ ,s_comm_svr_socket.water_timeout);
   	signal( SIGALRM, func_timeout_handler );
   	alarm( s_comm_svr_socket.water_timeout );
   	
   	if( setjmp( T_TmOutEnv ) != 0 ){/*��ʱ*/
   		CommLog(LOG , "[%s][%d] �ӽ���  ���׳�ʱ [%d]",__FILE__,__LINE__ ,timeout);
   		/*���س�ʱ��Ӧ��*/
   		memset(resp_code, 0x00, sizeof(resp_code)); 
   		memset(send_msg, 0x00, sizeof(send_msg));
   		
   		strncpy(resp_code,RESPCODE_TIMEOUT,sizeof(resp_code)-1);
   	  resp_code[sizeof(resp_code)-1]='\0';   
   	  
   		
   		/*���ĸ�ʽ����Ӧ��|Tuxedo������|���ݰ�*/
     	snprintf(send_msg,sizeof(send_msg),"%s$$",resp_code);
		  sendlen =strlen(send_msg);
		  
		  CommLog(LOG , "[%s][%d] ��Ӧ����send_msg[%s] ",__FILE__,__LINE__ ,send_msg);
   	  CommLog(LOG , "[%s][%d] ��Ӧ���ĳ���[%d] ",__FILE__,__LINE__ ,sendlen);
   	  ret = send_nbytes(conn,send_msg,sendlen);
   	  if(ret != sendlen){
   	     CommLog(LOG , "[%s][%d] ������Ӧ����ʧ��[%d],socket[%d] ",__FILE__,__LINE__ ,ret,conn);
		     return E_FAIL;
   	  }
		  CommLog(LOG , "[%s][%d] �ӽ���  ������� ",__FILE__,__LINE__ );
		  return E_FAIL;
	  }
    CommLog(LOG , "[%s][%d] �ӽ���  �������� ",__FILE__,__LINE__ );
   	
   	ret = process_parse(conn);
   	if(ret != E_OK){
   		CommLog(LOG , "[%s][%d] �ӽ��� ����ʧ��ret[%d] ",__FILE__,__LINE__ ,ret);
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
	  	CommLog(LOG , "[%s][%d] �ӽ��� begin,pid[%d]  ",__FILE__,__LINE__, getpid() );
	  	CommLog(LOG , "[%s][%d] �ӽ��̵��÷���  ",__FILE__,__LINE__ );
	  	
	  	process_main(connect);
	  	ret = sock_close( connect );
	  	CommLog(LOG , "[%s][%d] �ӽ��̹ر�����FD[%d]ret[%d]  ",__FILE__,__LINE__,connect,ret );
	  	CommLog(LOG , "[%s][%d] �ӽ����˳�exit()  ",__FILE__,__LINE__ );
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
	  unsigned long   cli_addr ;     /*�ͻ�ip*/
   	unsigned short  cli_port ;
   	struct  timeval tval ;
	  
	  CommLog(LOG , "[%s][%d]parent_proc begin,listen_fd[%d]" ,__FILE__,__LINE__,listen_fd); 
	  CommLog(LOG , "[%s][%d] ������ select��ʼ ����������listen_fd[%d]" ,__FILE__,__LINE__,listen_fd);  
	  while(1){
	      FD_ZERO(&rset);
	      FD_SET(listen_fd,&rset);	
	      tval.tv_sec  = 0 ;
		    tval.tv_usec = 100000  ;
	      rets = select(max_listen_fd+1,&rset,NULL,NULL,&tval) ;
	      	 
	      if(rets < 0 ){/* �ж� */
		    	CommLog(LOG , "[%s][%d] ������ select ����ֵ[%d]errno[%d] continue" ,__FILE__,__LINE__,rets,errno, strerror(errno)); 
		    	continue ;
		    }else if( rets == 0 ){ /* ��ʱ  */	
		    	/*CommLog(LOG , "[%s][%d] ������ select ��ʱ" ,__FILE__,__LINE__); */
		    	continue ;
		    }else{/**/
		    	 CommLog(LOG , "[%s][%d] ������ select ����,listen_fd[%d]" ,__FILE__,__LINE__,listen_fd); 
		    	 n_connect = tcp_accept (listen_fd, &cli_addr,&cli_port );
		    	 if( n_connect < 0 ){
			       CommLog(LOG , "[%s][%d] ������ accept connect is [%d] continue ",__FILE__,__LINE__, n_connect );
			       continue ;
		       }
		       CommLog(LOG , "[%s][%d] ������ accept connect is [%d]  ",__FILE__,__LINE__, n_connect );
           ret = fork_proc(n_connect);
           CommLog(LOG , "[%s][%d] �ӽ��� pid [%d],����ҵ����  ",__FILE__,__LINE__, ret );
		       ret = sock_close( n_connect );	
		       CommLog(LOG , "[%s][%d] ������ �ر� ����fd [%d]  ",__FILE__,__LINE__, n_connect );
		    }
		    CommLog(LOG , "[%s][%d] ������ select��ʼ������������listen_fd[%d]" ,__FILE__,__LINE__,listen_fd);  
	}
	
}


/*����������*/
void do_process(void)
{
  
  int		listen_fd = -1 ;/*������������*/
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
	/*��ȡ������Ϣ*/
	memset(&s_comm_svr_socket, 0x00, sizeof(struct comm_svr_socket));
	ret = get_comm_svr_cfg(cfg_path,&s_comm_svr_socket);
	if(ret != E_OK ){
	  CommLog(LOG , "[%s][%d] �ӽ����˳�exit" ,__FILE__,__LINE__); 	
	 	exit(-1);
	}
	snprintf (s_comm_svr_socket.tux_envstr,sizeof(s_comm_svr_socket.tux_envstr),"WSNADDR=//%s:%s",s_comm_svr_socket.water_ip,s_comm_svr_socket.water_port);
	bcl_trimall(s_comm_svr_socket.tux_envstr);
	
	CommLog(LOG , "[%s][%d]  ת���������˿�  [%d]" ,__FILE__,__LINE__,s_comm_svr_socket.comm_svr_port); 
	CommLog(LOG , "[%s][%d]  ����ˮ��˾IP    [%s]" ,__FILE__,__LINE__,s_comm_svr_socket.water_ip); 
	CommLog(LOG , "[%s][%d]  ����ˮ��˾�˿�  [%s]" ,__FILE__,__LINE__,s_comm_svr_socket.water_port); 
	CommLog(LOG , "[%s][%d]  tux_envstr      [%s]" ,__FILE__,__LINE__,s_comm_svr_socket.tux_envstr); 
	CommLog(LOG , "[%s][%d]  ��ʱʱ��        [%d]" ,__FILE__,__LINE__,s_comm_svr_socket.water_timeout); 
	
	ret = tcp_listener( &listen_fd, htonl( INADDR_ANY ), s_comm_svr_socket.comm_svr_port );
	if(ret != E_OK){
		CommLog(LOG , "[%s][%d]tcp_listener error ,s_comm_svr_socket.comm_svr_port[%d]" ,__FILE__,__LINE__,s_comm_svr_socket.comm_svr_port); 
		exit(-1);
	}
		
	parent_proc(listen_fd);	/*�����쳣���ӽ��̳ɹ���ʧ�ܺ�exit*/
	
}



int main(int argc,char **argv)
{
	
	pid_t    pid = -1;
	pid_t   wpid = -1;
	pid_t   cpid = -1;
	int     stat = 0;
	
	 /*�ػ�����*/
	 /* ת�ɼ�ؽ��� */
	run_daemon(getenv("HOME"));
	
	signal( SIGCLD,  SIG_IGN );	
	signal( SIGCHLD ,SIG_DFL );	
	
   /* ���������̣�ִ��ҵ���߼�*/
   pid=fork();
   if(pid <0 ){ 
   	fprintf(stderr,"fork() error\n");
		fflush(stderr);
		exit(-1);
   }else if( pid == 0 ){
	   	do_process();
   }
    
    /*����������,�����˳�������fork�ӽ���cpid*/
    while(1){
       while ( ( wpid = waitpid(-1, &stat, WNOHANG)) > 0 ) {
       	CommLog(LOG , "[%s][%d] ���������̣��ӽ����˳�exit������fork�ӽ���" ,__FILE__,__LINE__); 	
       	
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
		   CommLog(LOG , "[%s][%d] �ӽ���ȡ�˿ںŴ�  ",__FILE__,__LINE__ );
	   	 return -1 ;
	  }
	  srv_port = port_s;
	  CommLog(LOG , "[%s][%d] �ӽ���ȡ�˿ں�[%d]",__FILE__,__LINE__ ,srv_port);
   	/*���ó�ʱ*/
   	CommLog(LOG , "[%s][%d] �ӽ������ó�ʱ[%d]",__FILE__,__LINE__ ,s_comm_svr_socket.water_timeout);
   	signal( SIGALRM, func_timeout_handler );
   	alarm( s_comm_svr_socket.water_timeout );
   	
   	if( setjmp( T_TmOutEnv ) != 0 ){/*��ʱ*/
   		CommLog(LOG , "[%s][%d] �ӽ���  ���׳�ʱ [%d]",__FILE__,__LINE__ ,timeout);
   		/*���س�ʱ��Ӧ��*/
   		memset(resp_code, 0x00, sizeof(resp_code)); 
   		memset(send_msg, 0x00, sizeof(send_msg));
   		
   		strncpy(resp_code,RESPCODE_TIMEOUT,sizeof(resp_code)-1);
   	  resp_code[sizeof(resp_code)-1]='\0';   
   	  
   		sendlen =PKGHEAD_LENGTH + 4 +strlen(resp_code);
     	snprintf(send_msg,sizeof(send_msg),"%08d$%s$$$",sendlen-PKGHEAD_LENGTH,resp_code);
		  
		  CommLog(LOG , "[%s][%d] ��Ӧ����send_msg[%s] ",__FILE__,__LINE__ ,send_msg);
   	  CommLog(LOG , "[%s][%d] ��Ӧ���ĳ���[%d] ",__FILE__,__LINE__ ,sendlen);
   	  ret = send_nbytes(conn,send_msg,sendlen);
   	  if(ret != sendlen){
   	     CommLog(LOG , "[%s][%d] ������Ӧ����ʧ��[%d],socket[%d] ",__FILE__,__LINE__ ,ret,conn);
		     return E_FAIL;
   	  }
		  CommLog(LOG , "[%s][%d] �ӽ���  ������� ",__FILE__,__LINE__ );
		  return E_FAIL;
	  }
    CommLog(LOG , "[%s][%d] �ӽ���  �������� ",__FILE__,__LINE__ );
   	
   	ret = process_parse(conn);
   	if(ret != E_OK){
   		CommLog(LOG , "[%s][%d] �ӽ��� ����ʧ��ret[%d] ",__FILE__,__LINE__ ,ret);
   		return E_FAIL;
   	}
   	return E_OK;
} 

/*���ĸ�ʽ��8λ����$��Ӧ��$Tuxedo������$���ݰ�$
   	    ��ͷ��  8λ����,����ʱ����
   	    ���壺  $��Ӧ��$Tuxedo������$���ݰ�$
   	    ����:   ���峤��
   	    ��Ӧ��: 0 ͨѶ�ɹ� 1��ʱ 2 tuxedoͨѶʧ�� Ĭ����9
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
   	  CommLog(LOG , "[%s][%d] �ӽ��� ,����ͷ����8λrecv_head [%s]",__FILE__,__LINE__ ,recv_head);
		  return -1;
   	}
   	
   	msglen = atoi(recv_head);
   	
   	CommLog(LOG , "[%s][%d] �ӽ��� ,���ĸ�ʽ��8λ����$Tuxedo������$���ݰ�$����ͷ����ֻ��������",__FILE__,__LINE__ );
   	CommLog(LOG , "[%s][%d] �ӽ��� ,8λ����ͷrecv_head [%s],msglen[%d]",__FILE__,__LINE__ ,recv_head,msglen);
   	memset(recv_msg, 0x00, sizeof(recv_msg));
   	memcpy(recv_msg,recv_head,PKGHEAD_LENGTH);
   	ret = recvn(conn,recv_msg+PKGHEAD_LENGTH,msglen);
   	if(ret != msglen){
   		CommLog(LOG , "[%s][%d] �ӽ��� ,�������ʽ��,���Ȳ���[%d],ʵ��[%d],recv_msg[%s] ",__FILE__,__LINE__ ,msglen,ret,recv_msg);
		  return -1;
   	}
   	recvlen = PKGHEAD_LENGTH + msglen;
   	CommLog(LOG , "[%s][%d] ����recv_msg[%s] ",__FILE__,__LINE__ ,recv_msg);
   	CommLog(LOG , "[%s][%d] ���ĳ���[%d] ",__FILE__,__LINE__ ,recvlen);
   	
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
    
    CommLog(LOG , "[%s][%d] Tuxedo������          [%s] ",__FILE__,__LINE__ ,tran_code);
    CommLog(LOG , "[%s][%d] Tuxedo�����������ݰ�  [%s] ",__FILE__,__LINE__ ,request_data);
    CommLog(LOG , "[%s][%d] Tuxedo���׻�������    [%s] ",__FILE__,__LINE__ ,s_comm_svr_socket.tux_envstr);
   	
   	ret = comm_tux_tpcall(tran_code,request_data, response_data,s_comm_svr_socket.tux_envstr);
   	CommLog(LOG , "[%s][%d] Tuxedo������Ӧ���ݰ�  [%s] ,����[%d]",__FILE__,__LINE__ ,response_data,strlen(response_data));
   	if(ret != E_OK){
   	  	/*TuxedoͨѶʧ��*/
   	  	memset(resp_code, 0x00, sizeof(resp_code));
   	    strncpy(resp_code,RESPCODE_TUXEDO_ERR,sizeof(resp_code)-1);
        resp_code[sizeof(resp_code)-1]='\0';   	
   	}else{/*TuxedoͨѶ�ɹ�*/
   		  memset(resp_code, 0x00, sizeof(resp_code));
   	    strncpy(resp_code,RESPCODE_OK,sizeof(resp_code)-1);
   	    resp_code[sizeof(resp_code)-1]='\0';   	
   	}
   	
   	/*���ĸ�ʽ��8λ����$��Ӧ��$Tuxedo������$���ݰ�$*/
   	sendlen =PKGHEAD_LENGTH + 4 +strlen(resp_code) +strlen(tran_code) +strlen(response_data);
   	memset(send_msg, 0x00, sizeof(send_msg));
   	snprintf(send_msg,sizeof(send_msg),"%08d$%s$%s$%s$",sendlen-PKGHEAD_LENGTH,resp_code,tran_code,response_data);
   	
   	CommLog(LOG , "[%s][%d] ��Ӧ����send_msg[%s] ",__FILE__,__LINE__ ,send_msg);
   	CommLog(LOG , "[%s][%d] ��Ӧ���ĳ���[%d] ",__FILE__,__LINE__ ,sendlen);
   	ret = send_nbytes(conn,send_msg,sendlen);
   	if(ret != sendlen){
   	  CommLog(LOG , "[%s][%d] ������Ӧ����ʧ��[%d],socket[%d] ",__FILE__,__LINE__ ,ret,conn);
		  return E_FAIL;
   	}
   	
   	return E_OK;
	
}
