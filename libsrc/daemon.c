#include "wrapunix.h"


int   run_daemon(char *workdir)
{
	pid_t   pid ;
	    
	if ( (pid=fork())<0 ){
		fprintf(stderr,"fork() error\n");
		fflush(stderr);
		exit(-1);
	}else if (pid != 0 ){ 
		exit(0);
	}

	setsid();       
	signal( SIGHUP,  SIG_IGN );	
	if ((pid=fork())<0 ){
		fprintf(stderr,"fork() error\n");
		fflush(stderr);
		exit(-1) ;
	}else if (pid!=0){
		exit(0);
	}
	umask(0);			
	if(workdir != NULL )
		chdir(workdir);
	
	return 0;
}
