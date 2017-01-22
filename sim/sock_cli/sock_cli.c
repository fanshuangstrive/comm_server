/* client.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>


#define MAXLINE  1024
#define SERV_PORT 9999
#define SERV_IP "128.5.36.102"




int read_file(FILE *fp, char *sndmsg)
{
   char msg_tmp[1024];
   
   memset(msg_tmp,0x00,sizeof(msg_tmp));
   while ( fgets(msg_tmp,sizeof(msg_tmp),fp) != NULL)
   {
       strcat(sndmsg,msg_tmp);
       memset(msg_tmp,0x00,sizeof(msg_tmp));
   }
   
   return 0;
}

extern int errno;
int readn(int fd , void * vptr , size_t n)
{
    ssize_t nleft;
    ssize_t nread;
    char    *ptr;

    ptr = vptr;
    nleft = n ;
    while(nleft > 0){
        if((nread = read(fd , ptr , nleft)) < 0){
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

int main(int argc, char *argv[])
{
  struct sockaddr_in servaddr;
  char buf[MAXLINE];
  char sendbuf[MAXLINE];
  char sendbuf1[MAXLINE];
  int sockfd, n;
  char *str;
	FILE *fp;
	char *p_ip = NULL;
	char *p_port = NULL;
	char *p_filepath = NULL;
	int  ret = 0;
	unsigned short uport = 0;
    
  if (argc != 4) {
    fprintf(stderr,"usage: ./client  IP PORT 文件名\n");
    exit(1);
  }
  
  p_ip= argv[1];
  p_port = argv[2];
  p_filepath = argv[3];
	memset(sendbuf,0,sizeof(sendbuf));
	memset(sendbuf1,0,sizeof(sendbuf1));

 	printf("IP[%s]\n",p_ip);
 	printf("PORT[%s]\n",p_port);
 	printf("File[%s]\n",p_filepath);



	fp = fopen(p_filepath,"r");
  if(fp == NULL){
    printf("%s|%d| 文件不存在[%s]\n",__FILE__,__LINE__,argv[1]);
    return 0;
  }
  read_file(fp,sendbuf);
  fclose(fp);

	printf("发送的报文str[%s]\n",sendbuf);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;

 
  ret = inet_pton(AF_INET, p_ip, &servaddr.sin_addr);
  if(ret != 1){
  	fprintf(stdout , "[%s][%d] inet_pton:errno[%d] inet_pton[%s]" ,__FILE__,__LINE__,errno, strerror(errno)); 
    return -1;	
  }
  
  uport = atoi(p_port);
  
  servaddr.sin_port = htons(uport);

  ret = connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr));
  if( ret != 0 ){
  	fprintf(stdout , "[%s][%d] connect:errno[%d] inet_pton[%s]" ,__FILE__,__LINE__,errno, strerror(errno)); 
    return -1;	
  }
  
  write(sockfd, sendbuf, strlen(sendbuf));
  perror("write");

  memset(buf,0x00,sizeof(buf));
  n = readn(sockfd, buf, MAXLINE);
  perror("read");  
/*
  printf("Response from server:\n"); 
  write(STDOUT_FILENO, buf, n);
*/
  printf("\n\n\n返回的报文[%s],[%d]\n",buf,n);
  close(sockfd);
  return 0;
}



