#include "wrapunix.h"




/*日志文件每天一个*/
int CommLog(char *logname , char *strfmt,...)
{
        va_list  ap;
        int     len , num;
        FILE    *fp;
        char    fname[128];
        struct tm       *pcNow;
        time_t  cNow;
        struct timeval ttt ;

        fp=NULL;
        len=num=0;
        memset(fname , 0x00 , sizeof(fname));
        time(&cNow);
        gettimeofday(&ttt,NULL);
        pcNow=localtime(&cNow);
        

        sprintf(fname , "%s/%s.%04d%02d%02d",getenv("HOME"),\
                  logname,pcNow->tm_year+1900,pcNow->tm_mon+1,pcNow->tm_mday);
        fp = fopen(fname , "a");
        if( fp == NULL )
        {
                printf("file open err\n");
                return E_FAIL;
        }

        
        fprintf(fp,"%04d%02d%02d|%02d%02d%02d%03ld|%05d|",\
                pcNow->tm_year+1900,\
                pcNow->tm_mon+1,\
                pcNow->tm_mday,\
                pcNow->tm_hour,\
                pcNow->tm_min,\
                pcNow->tm_sec,\
                ttt.tv_usec/1000,\
               getpid());
        va_start(ap , strfmt);

        vfprintf( fp , strfmt , ap);
        fprintf(fp , "|\n" );
        fclose(fp);
        va_end(ap);
       
        return E_OK;
}


/*日志文件一个*/
int CommLog_signal(char *logname , char *strfmt,...)
{
        va_list  ap;
        int     len , num;
        FILE    *fp;
        char    fname[128];
        struct tm       *pcNow;
        time_t  cNow;
        struct timeval ttt ;

        fp=NULL;
        len=num=0;
        memset(fname , 0x00 , sizeof(fname));

        sprintf(fname , "%s/%s",getenv("HOME"),logname);
        fp = fopen(fname , "a");
        if( fp == NULL )
        {
                printf("file open err\n");
                return E_FAIL;
        }

        time(&cNow);
        gettimeofday(&ttt,NULL);
        pcNow=localtime(&cNow);
        fprintf(fp,"%04d%02d%02d|%02d%02d%02d%03ld|%05d|",\
                pcNow->tm_year+1900,\
                pcNow->tm_mon+1,\
                pcNow->tm_mday,\
                pcNow->tm_hour,\
                pcNow->tm_min,\
                pcNow->tm_sec,\
                ttt.tv_usec/1000,\
               getpid());
        va_start(ap , strfmt);

        vfprintf( fp , strfmt , ap);
        fprintf(fp , "|\n" );
        fclose(fp);
        va_end(ap);
       
        return E_OK;
}


