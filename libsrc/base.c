#include "wrapunix.h"


 char    *get_str(char *source,int *m,char *dlmt)
{
	static char    des_str[2049];
	char   *p;
	int    i;

	i=1;
	memset(des_str,0x00,sizeof(des_str));
	if(strlen(source) == 0)
		return( des_str );
	p = source ;
	while(i < *m){
		if(strncmp(p,dlmt,strlen(dlmt))==0){ 
			i++;
			p += strlen(dlmt);
		}
		else
			p++;
		if( *p == '\0' )
			return(des_str);
	}
	for(i=0;strncmp(p,dlmt,strlen(dlmt))!=0;i++){
		if( i > 2048 ) 
			break;
		des_str[i]=*p;
		if( *p == '\0')
			break;
		p++;
	}
	des_str[i]='\0';
	return(des_str);
}
 char *bcl_trimall( char *str )
{
    char *pstart, *ptr;

    if ( str == NULL )
        return NULL;

    pstart = ptr = str;

    while ( *ptr != 0x0 )
        if ( *( ( unsigned char * )ptr ) == 0x20 )
            ptr ++;
        else
            *str ++ = *ptr ++;

    *str = 0x0;

    return pstart;
}

char *bcl_ltrim( char *str )
{
    char	*pstart, *ptr;

    if ( str == NULL )
        return NULL;

    pstart = ptr = str;

    while ( *( ( unsigned char * )ptr ) == 0x20 || *ptr == '\t' || \
            *ptr == '\r' || *ptr == '\n' )
        ptr ++;

    while ( *ptr != 0x0 )
        *str ++ = *ptr ++;

    *str = 0x0;

    return pstart;
}
char *bcl_rtrim( char *str )
{
    char	*ptr;

    if ( str == NULL )
        return NULL;

    ptr = str + strlen( str ) - 1;

    while ( ptr >= str && ( *( ( unsigned char * )ptr ) == 0x20 || \
            *ptr == '\t' ||  *ptr == '\r' || *ptr == '\n' ))
        *ptr -- = 0x0;

    return str;
} 


/**
入参：
  filepath: 文件路径
  name    : 参数名
出参:
  value   :值

**/
int get_cfg_value( char* filepath ,  char *name , char *value )
{
	 int ret = E_FAIL;
   FILE *fp = NULL;	
   char buf[MAX_LINE_LEGTH];
   char tmp_name[MAX_LINE_LEGTH];
   
   if( (filepath == NULL) || (name == NULL)){
     return E_OK;	
   }
   
   fp = fopen(filepath,"r");
   if( fp == NULL){
     	CommLog(LOG,"[%s][%d] fopen error,[%s],filepath[%s]" ,__FILE__,__LINE__,strerror(errno),filepath);
   	  return E_FAIL;
   }
   
   memset(buf, 0x00,sizeof(buf));
   memset(tmp_name, 0x00,sizeof(tmp_name));
   ret = E_FAIL;
   while( fgets(buf,sizeof(buf),fp)!= NULL){
   	 bcl_rtrim( buf );
     bcl_ltrim( buf );
   	 
   	 if ( buf[ 0 ] == '#'|| buf[ 0 ] == '/'|| buf[ 0 ] == '*'  || buf[ 0 ] == '\n' || strlen( buf ) == 0 )
            continue;
            
     if ( strncmp( buf, name, strlen(name) ) == 0){
            sscanf( buf, "%s %s", tmp_name, value);
            ret = E_OK;
            break; 
     }  
            
   	 memset(buf, 0x00,sizeof(buf));
   	 memset(tmp_name, 0x00,sizeof(tmp_name));
   }
   
   fclose(fp);
   return ret;	
} 
