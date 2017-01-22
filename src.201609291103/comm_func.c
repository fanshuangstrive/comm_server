
#include "comm_func.h"


int comm_tux_tpcall(char*servicename, char *request_buf,char * response_buf,char *envstr)
{
  char  *sendbuf=NULL, *recvbuf=NULL;
  int ret = 0;
  long recvlen;
 
  CommLog(LOG , "[%s][%d] Tuxedo envstr=[%s]",__FILE__,__LINE__,envstr);
  
  tuxputenv(envstr);

  ret = tpinit( (TPINIT*)NULL );
  if( ret== E_FAIL) {
      CommLog(LOG , "[%s][%d] Tuxedo TPCALLNO=[%s]",__FILE__,__LINE__,tpstrerror(tperrno));
    
      return E_FAIL;
  }

  sendbuf=(char*)tpalloc( "STRING", NULL, strlen(request_buf)+1 );
  if(sendbuf==NULL) {
          CommLog(LOG , "[%s][%d] Tuxedo TPCALLNO=[%s]",__FILE__,__LINE__,tpstrerror(tperrno));
          tpterm();
          return E_FAIL ;
  }
  recvbuf = (char*)tpalloc( "STRING", NULL, MAXBUF_LEN );
  if( recvbuf == NULL){
          CommLog(LOG , "[%s][%d] Tuxedo TPCALLNO=[%s]",__FILE__,__LINE__,tpstrerror(tperrno));
          tpfree( sendbuf );
          tpterm();
          return  E_FAIL;
  }
  
  strcpy(sendbuf,request_buf);
  
  ret = tpcall(servicename, (char *)sendbuf,0, (char **)&recvbuf, &recvlen, (long)0);
  if ( ret < 0 ) {/*失败*/
     CommLog(LOG , "[%s][%d] Tuxedo tpcall error: ret [%d] ",__FILE__,__LINE__,ret);
     CommLog(LOG , "[%s][%d] Tuxedo tpcall error: tpurcode=[%d](an application defined value that was sent as part of  tpreturn()) ",__FILE__,__LINE__,tpurcode);
     CommLog(LOG , "[%s][%d] Tuxedo tpcall error: tperrno[%d],[%s], ",__FILE__,__LINE__,tperrno,tpstrerror(tperrno));
  }else{/*成功*/
  	 CommLog(LOG , "[%s][%d] Tuxedo tpcall success: ret [%d] ",__FILE__,__LINE__,ret);
     CommLog(LOG , "[%s][%d] Tuxedo tpcall success: tpurcode=[%d] ",__FILE__,__LINE__,tpurcode);/*(an application defined value that was sent as part of  tpreturn()*/
     CommLog(LOG , "[%s][%d] Tuxedo tpcall success: tperrno[%d],[%s]",__FILE__,__LINE__,tperrno,tpstrerror(tperrno));
  }
 
  CommLog(LOG , "[%s][%d] Tuxedo 响应数据包[%s],长度[%d]" ,__FILE__,__LINE__,recvbuf,recvlen); 
  
  if(recvlen > (MAXBUF_LEN-100) ){
  	 CommLog(LOG , "[%s][%d] Tuxedo 返回报文太长,recvlen[%d],最大[%d]" ,__FILE__,__LINE__,recvlen,MAXBUF_LEN-100); 
  	 tpfree(recvbuf);
     tpfree(sendbuf);
     tpterm();
     return  E_FAIL;
  }
  strncpy(response_buf,recvbuf,recvlen);
 
  tpfree(recvbuf);
  tpfree(sendbuf);
  tpterm();
  
  return E_OK;
} 




 


int get_comm_svr_cfg(char * cfg_path,struct comm_svr_socket *p)
{
	char value[256];
	int ret ;
	
	struct comm_svr_socket s_comm;
	
	memset(&s_comm,0x00,sizeof(struct comm_svr_socket ));
	memset(value,0x00,sizeof(value));
  ret = get_cfg_value(cfg_path, "COMM_SVR_PORT" , value );
  if(ret != E_OK){
  	CommLog(LOG , "[%s][%d]get_cfg_value error 没有[COMM_SVR_PORT],cfg_path[%s]" ,__FILE__,__LINE__,cfg_path); 
  	return E_FAIL;
  }
	s_comm.comm_svr_port = atoi(value);
	
	memset(value,0x00,sizeof(value));
  ret = get_cfg_value(cfg_path, "WATER_IP" , value );
  if(ret != E_OK){
  	CommLog(LOG , "[%s][%d]get_cfg_value error 没有[WATER_IP],cfg_path[%s]" ,__FILE__,__LINE__,cfg_path); 
  	return E_FAIL;
  }
  snprintf(s_comm.water_ip,sizeof(s_comm.water_ip),"%s",value);
  
  
  memset(value,0x00,sizeof(value));
  ret = get_cfg_value(cfg_path, "WATER_PORT" , value );
  if(ret != E_OK){
  	CommLog(LOG , "[%s][%d]get_cfg_value error 没有[WATER_PORT],cfg_path[%s]" ,__FILE__,__LINE__,cfg_path); 
  	return E_FAIL;
  }
  snprintf(s_comm.water_port,sizeof(s_comm.water_port),"%s",value);
  
  
 	memset(value,0x00,sizeof(value));
  ret = get_cfg_value(cfg_path, "WATER_TIMEOUT" , value );
  if(ret != E_OK){
  	CommLog(LOG , "[%s][%d]get_cfg_value error 没有[WATER_TIMEOUT],cfg_path[%s]" ,__FILE__,__LINE__,cfg_path); 
  	return E_FAIL;
  }
  s_comm.water_timeout = atoi(value);
  
  memcpy(p, &s_comm,sizeof(struct comm_svr_socket));
  
	
	return E_OK;
}
