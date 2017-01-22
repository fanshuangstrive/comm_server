#include "wrapunix.h"


#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif
int setnbio( int fd, int flg )
{
	int		optval;

	if ( flg == TRUE ) 
		optval = 1;
	else
		optval = 0;

	if ( ioctl( fd, FIONBIO, &optval ) == -1 ) {
		CommLog(LOG ,  "[%s][%d]ioctl() error,[%s]" ,__FILE__,__LINE__,strerror(errno));
		return -1;
	}
	  
	return 0;
}
int send_nbytes( int sock, void *buf, int len )
{
    int left=len, cnt, sended=0;
	
    setnbio( sock, FALSE );
    while ( left > 0 ) {
        errno = 0;
        if ( ( cnt = send( sock, buf+sended, left, 0 ) ) < 0 ) {
			     if ( errno == EINTR ){
				      continue;
		       }
           CommLog(LOG ,  "[%s][%d]send() error,[%s]" ,__FILE__,__LINE__,strerror(errno));
           return -1;
        }
        left   -= cnt;
        sended += cnt;
    }

	return sended;
}
int bcl_bytesreadable( int sock )
{
    int		value=0;

    if ( ioctl( sock, FIONREAD, &value ) < 0 ) {
         /*bclerreg( E_OSCALL, __FILE__, __LINE__, "ioctl()" );*/
        return E_FAIL;
    }

    return value;
}
int  bcl_select( int _nfd, fd_set *_rfd, fd_set *_wfd,
            fd_set *_efd, struct timeval *_tv )
{
    int  ret;
    ret = select((SELECT_ARG1)_nfd, (SELECT_ARG234)_rfd,
           (SELECT_ARG234)_wfd, (SELECT_ARG234)_efd, (SELECT_ARG5)_tv);
    return  ret;
}

int bcl_sockisreadable( int sock, int tmval )
{
    fd_set	readmask;
    struct	timeval tv, *ptv = NULL;

    memset( &tv, 0, sizeof(struct timeval) );
    if ( tmval >= 0 ) {
        tv.tv_sec       = tmval;
        ptv             = &tv;
    }

    for ( ; ; ) {
        FD_ZERO( &readmask );
        FD_SET( sock, &readmask );
        switch ( bcl_select(sock+1, &readmask, 0, 0, ptv) ) {
            case 0: /* socket normal */
                return SS_TIMEDOUT;
            case -1: /* socket abnormal */
                if ( errno == EINTR )
                    continue;
                /*bclerreg( E_OSCALL, __FILE__, __LINE__, "select()" );*/
                return SS_BROKEN;
            default: /* socket has event occur */
                if ( FD_ISSET( sock, &readmask ) ) {

#ifndef __INTERIX

                    if( bcl_bytesreadable( sock ) <= 0 ) {
                        /*bclerreg( E_OSCALL, __FILE__, __LINE__, "bcl_getsockerr()" );*/
                        return SS_BROKEN;
                    }

#endif

                    return SS_READABLE;
                }
                /*
                return SS_BROKEN;
                */
        }
    }
}

int  bcl_getrcvbuflen( int sock )
{
    int		val;
    int   	vallen;

    vallen = sizeof(val);
 
    if ( getsockopt( sock, SOL_SOCKET, SO_RCVBUF, &val, &vallen ) < 0 ) {
        if ( errno == ENOTSOCK )
            val = DEFAULTBUFLEN;
        else
            val = MINSOCKBUFLEN;
    }

    return (val<=0)?MINSOCKBUFLEN:val;
}
int recv_nbytes( int sock, void *buf, int len, int tmot )
{
    int     left, cnt, readed=0;
    int     r_buf_len=0;
    
    left = len ; 
    r_buf_len = bcl_getrcvbuflen( sock );
   
    while ( left > 0 ){
        switch ( bcl_sockisreadable(sock,tmot )) {
            case SS_READABLE:
		CommLog(LOG ,   "[%s][%d] sockisreadable() returned SS_READABLE" ,__FILE__,__LINE__);
                if ( (cnt=recv(sock, (char *)buf+readed, \
                                     min(left,r_buf_len),0)) <= 0 ) {
                    if ( errno == EINTR )
                        continue;
		    CommLog(LOG ,   "[%s][%d] recv()" ,__FILE__,__LINE__);
                    /*bclerrlog( E_OSCALL, __FILE__, __LINE__, "recv()" );*/
                    return E_FAIL;
                }
                else if( cnt == 0 )
                    return readed ;    
                left   -= cnt;
                readed += cnt;
                break ;
            case SS_TIMEDOUT:
		CommLog(LOG ,   "[%s][%d] sockisreadable() returned SS_TIMEDOUT" ,__FILE__,__LINE__);
               /* bclerrlog( E_FUNCALL, __FILE__, __LINE__, \
                                "sockisreadable() returned SS_TIMEDOUT" );
		*/
                if(readed > 0)
                    return readed;
                else
                    return E_FAIL;
                break ;    
            case SS_BROKEN:
		CommLog(LOG ,   "[%s][%d] sockisreadable() returned SS_BROKEN" ,__FILE__,__LINE__);
		/*
                bclerrlog( E_FUNCALL, __FILE__, __LINE__, \
                                "sockisreadable() returned SS_BROKEN" );
		*/
                if(readed > 0)
                    return readed;
                else
                    return E_FAIL;
        }

    }
    
    return readed;
}

int recvn(int fd , void * vptr , size_t n)
{   
    ssize_t nleft;
    ssize_t nread;
    char    *ptr;
    
    ptr = vptr;
    nleft = n ;
    while(nleft > 0){
	CommLog(LOG ,   "[%s][%d] recvn[%d]" ,__FILE__,__LINE__,nleft);
        if((nread = recv(fd , ptr , nleft,0)) < 0){ 
            if(errno == EINTR)
                nread = 0;           
            else    
                return(-1);
        }else if (nread == 0)
            break;  
        nleft -= nread;
        ptr += nread;
    }
    return(n - nleft);
}
int recvn_timeout(int fd , void * vptr , size_t n)
{
    ssize_t nleft;
    ssize_t nread;
    char    *ptr;

    ptr = vptr;
    nleft = n ;
    while(nleft > 0){
        if((nread = recv(fd , ptr , nleft,0)) < 0){
            if(errno == EINTR)
                nread = 0;
            else
                return(-1);
        }else if (nread == 0)
            break;
        nleft -= nread;
        ptr += nread;
    }
    return(n - nleft);
}
int  tcp_listener( int *sockid,unsigned long _addr,unsigned short   _port )
{
    struct      sockaddr_in s_addr_in;
#if 0
    struct      linger linger;
#endif
    int         _sockid;
    int         optval=1;

	if ( (_sockid=socket( AF_INET, SOCK_STREAM, 0 )) == -1 ) {
		CommLog(LOG ,   "[%s][%d]socket() error,[%s]" ,__FILE__,__LINE__,strerror(errno));
		return -1;
	}

	memset( &s_addr_in, 0x0, sizeof( s_addr_in ) );

	s_addr_in.sin_family      = AF_INET;
	s_addr_in.sin_port        = htons( _port );
	s_addr_in.sin_addr.s_addr = _addr;

	setsockopt( _sockid, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int) );

#if 0
	linger.l_onoff  = 1;
	linger.l_linger = 10;
	setsockopt( _sockid, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger) );
#endif

	if ( bind( _sockid, (void *)&s_addr_in, sizeof( s_addr_in ) ) == -1 ) {
        CommLog(LOG ,  "[%s][%d]bind() error,[%s]" ,__FILE__,__LINE__,strerror(errno));
        sock_close( _sockid );
        return -1;
    }

    if ( listen( _sockid, SOMAXCONN ) == -1 ) {
        CommLog(LOG ,  "[%s][%d]listen() error,[%s]" ,__FILE__,__LINE__,strerror(errno));
        sock_close( _sockid );
        return -1;
    }

    /* 对于exec的子进程，监听fd自动关闭 */
    fcntl( _sockid, F_SETFD, 1 );
    *sockid = _sockid;

	return 0;
}
int  tcp_accept( int sock, unsigned long  *_peeraddr, unsigned short *_peerport )
{
	int		new_sock;
	struct	sockaddr_in peeraddr_in;
	unsigned int   	addr_len;

	addr_len = sizeof( struct sockaddr_in );
	memset( &peeraddr_in, 0x00, addr_len );

	for ( ; ; ) {   /* Loop if interrupted by signal */
		errno = 0;
		if ( ( new_sock = accept( sock, (void *)&peeraddr_in,&addr_len )) == -1 ) {
             if ( errno == EINTR )
                 continue;
              CommLog(LOG ,  "[%s][%d]accept() error,[%s]" ,__FILE__,__LINE__,strerror(errno));
             return -1;
        }
        break;
    }

    *_peeraddr = peeraddr_in.sin_addr.s_addr;
    *_peerport = peeraddr_in.sin_port;

    return( new_sock );
}


/**********取本地端口号**************/
int  get_local_port(int socket,u_short *port){
  	
  struct sockaddr_in sa_in;
  u_int  len ;
  int    ret ;
  
  memset(&sa_in,0x00,sizeof(struct sockaddr_in));
  len = sizeof(sa_in) ;
  
  ret = getsockname(socket, (struct sockaddr *)&sa_in, &len);
  if( ret < 0){
      CommLog( LOG, "FILE [%s] line %d getsockname error [%s]",__FILE__,__LINE__ ,strerror(errno));
      return E_FAIL ;
  }
  *port = ntohs(sa_in.sin_port);

   return E_OK ;
}
  
  
  
int sock_close( int sock )
{
	return close( sock );
}
