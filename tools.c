/*
 *  tools.c
 *  click-server
 *
 * Copyright 2010 (C) Alexandre Kalendarev 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef __CLICKER_TOOLS__
#define __CLICKER_TOOLS__
#include "clicker.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <cstdarg>
#include <evhttp.h>
#include <event.h>
#include "ini_parse.c"
#include "http_handler.cpp"

extern struct evhttp *httpd;

void
http_handler(struct evhttp_request *req, void *arg);

extern ctx Ctx;

void 
log_close(ctx * Ctx) {
	close(Ctx->log_fd);
	close(Ctx->err_fd);
//	if (res <1 ) {
//		perror("can't close log file");
//	} 
}
	
void 
tolog( ctx * Ctx,  int type, const char * fmt, ...) {
	short err=0;
	if (strlen(fmt)>580) {
// 		printf( "the log message is very long");
		err = 1;
	}
		
//	printf( "log_level=%d type=%d\n", Ctx->log_level ,type );
	if (Ctx->log_level < type) return;
	
	const time_t log_time=time(0);
	struct tm ts;
	localtime_r(&log_time, &ts);
	

	char * str_type=NULL;
	switch (type) {
		case LOG_NOTICE  : {str_type=strdup("NOTICE "); break;}
		case LOG_STDERR  : {str_type=strdup("STDERR "); break;}
		case LOG_CONTENT : {str_type=strdup("CONTENT");break;}
		case LOG_WARNING : {str_type=strdup("WARNING");break;}
		case LOG_ALERT   : {str_type=strdup("ALERT  ");break;}
		case LOG_ERROR   : {str_type=strdup("ERROR  ");break;}
		case LOG_DEBUG   : {str_type=strdup("DEBUG  ");break;}
		default			 : {str_type=strdup("-------");break;}
	}
		
	if (!str_type) {
		perror("strdup error");
		close(Ctx->err_fd);
		exit(1);	
	}	
	
	char buff [512];	
	bzero(buff,512);
	char * p;
	
	sprintf(buff , "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d %s:",1900+ts.tm_year,ts.tm_mon, ts.tm_mday, ts.tm_hour,ts.tm_min, ts.tm_sec,str_type);
	free(str_type);
	
	size_t len = strlen(buff);
	p=buff+len;
	
	char arg_c, * arg_s;
	int arg_d; 
	va_list ap;
	
	char * pfmt = (char *)fmt;
	len = strlen(fmt);

	va_start(ap, fmt);
	int len2;
	
	while( len--) {
		*p++ = *pfmt++;
		if ( *pfmt != '%' ) {
			continue;
		}
		
		pfmt++;
					
		switch(*pfmt) {
		  case 's':	
			arg_s=va_arg(ap, char *);
			len2=sprintf(p, "%s",arg_s);
			p+=len2;
			break;
		  case 'd':    
			arg_d=va_arg(ap, int);
			len2=sprintf(p, "%d",arg_d);
			p+=len2;
			break;
		  case 'x':    
			arg_d=va_arg(ap, int);
			len2=sprintf(p, "%x",arg_d);
			p+=len2;
			break;
		}
		pfmt++;
		len -=2;
	}

	va_end(ap);
	
	len=strlen(buff);
	*(buff+len)='\n';

//	printf("len=%d '%s'", len, buff);							
		
	len2 = write(Ctx->err_fd, buff, len+1);		
	
	if (len2 < 0) {
		fprintf(stderr, "writed %d, error %s\n",len2, strerror(errno));
		close(Ctx->err_fd);
		exit(1);
	} 

}

void 
toAccessLog( ctx * Ctx, const char * fmt, ...) {

	short err=0;
	if (strlen(fmt)>580) {
// 		printf( "the log message is very long");
		err = 1;
	}
		
//	printf( "log_level=%d type=%d\n", Ctx->log_level ,type );
	const time_t log_time=time(0);
	struct tm ts;
	localtime_r(&log_time,&ts);
	
	char buff [512];	
	bzero(buff,512);
	char * p;
	
	sprintf(buff , "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d\t",1900+ts.tm_year,ts.tm_mon, ts.tm_mday, ts.tm_hour,ts.tm_min, ts.tm_sec);

	size_t len = strlen(buff);
	p=buff+len;
	
	char arg_c, * arg_s;
	int arg_d; 
	va_list ap;
	
	char * pfmt = (char *)fmt;
	len = strlen(fmt);

	va_start(ap, fmt);
	int len2;
	
	while( len--) {
		*p++ = *pfmt++;
		if ( *pfmt != '%' ) {
			continue;
		}
		
		pfmt++;
					
		switch(*pfmt) {
		  case 's':	
			arg_s=va_arg(ap, char *);
			len2=sprintf(p, "%s",arg_s);
			p+=len2;
			break;
		  case 'd':    
			arg_d=va_arg(ap, int);
			len2=sprintf(p, "%d",arg_d);
			p+=len2;
			break;
		  case 'x':    
			arg_d=va_arg(ap, int);
			len2=sprintf(p, "%x",arg_d);
			p+=len2;
			break;
		}
		pfmt++;
		len -=2;
	}

	va_end(ap);
	
	len=strlen(buff);
	*(buff+len)='\n';

//	printf("len=%d '%s'", len, buff);							
		
	len2 = write(Ctx->log_fd, buff, len+1);		
	
	if (len2 < 0) {
		fprintf(stderr, "writed %d, error %s\n",len2, strerror(errno));
		close(Ctx->log_fd);
		exit(1);
	} 

	

}

int pid_read(ctx * Ctx) {
	FILE * fd = fopen(Ctx->pidfile, "r" );
	if (!fd) {
		perror("pid file not found");
		return -1;
	}

	char buf[16];
	bzero(buf,16);
	
	int res = fread(buf, 16,1, fd);
	if (res < 0){
		perror("can't read pid file");
		fclose(fd);
		return -1;	
	}
	
	fclose(fd);
	return atoi(buf);
	
}

int pid_create(ctx * Ctx) {
	Ctx->pidfd = open(Ctx->pidfile, O_CREAT | O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH );
	if (!Ctx->pidfd) {
		tolog(Ctx, LOG_ERROR,  "can't open pid file %s", strerror(errno));
//		perror("can't create pid file");
		return 1;
	}
	
	char buf[16]; 
	bzero(buf,16);
	sprintf(buf,"%ld",(long)getpid());	
	if (write( Ctx->pidfd,buf, strlen(buf)+1) < 1) {
		close(Ctx->pidfd);
		tolog(Ctx, LOG_ERROR, "can't write to pid file %s" , strerror(errno));
//		perror("can't write to pid file");
		return 1;
	} 
	
	return 0;
}

int pid_delete(ctx * Ctx) {
	if ( close(Ctx->pidfd) < 0){
		tolog(Ctx, LOG_ERROR, "can't close pid file %s", strerror(errno));
//		perror("can't close pid file");
	}
	
	size_t res = remove(Ctx->pidfile);
	if (res != 0) {
		tolog(Ctx, LOG_ERROR, "can't remove pid file %d" , strerror(errno));
	}
}

int pid_exist(ctx * Ctx) {
	struct stat st;
	if (!stat(Ctx->pidfile,&st)) {
		return 1;		
	}
	
	if (errno==ENOENT)
		return 0;
		
	return 1;				
}

int 
demonize(ctx * Ctx){

	int  fd;

	for (int i = 0; i < 1024; i++)
        close(i);
/*	int i;
	struct rlimit       rl;
	
	if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
        perror("can't get file limit");
	if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (i = 0; i < rl.rlim_max; i++)
        close(i);
*/

    switch (fork()) {
    case -1:
        perror("demonize fork failed");
        return -1;

    case 0:
        break;

    default:
		free_ctx(Ctx);
        exit(0);
    }

    int pid = getpid();

    if (setsid() == -1) {
//		tolog(Ctx, LOG_ERROR, "setsid() failed");
		perror("setsid() failed");        
        return -1;
    }

    umask(0);

    fd = open("/dev/null", O_RDWR);
    if (fd == -1) {
//		tolog("open(\"/dev/null\") failed", LOG_ERROR, Ctx);
		perror("open(\"/dev/null\") failed");        
        return -1;
    }

    if (dup2(fd, STDIN_FILENO) == -1) {
//		tolog("dup2(STDIN) failed", LOG_ERROR, Ctx);
		perror("dup2(STDIN) failed");        
        return -1;
    }

    if (dup2(fd, STDOUT_FILENO) == -1) {
//		tolog("dup2(STDOUT) failed", LOG_ERROR, Ctx);
		perror("dup2(STDOUT) failed");        		
        return -1;
    }

    if (dup2(fd, STDERR_FILENO) == -1) {
		tolog(Ctx, LOG_ERROR, "dup2(STDERR) failed", "");
//        logs << "dup2(STDERR) failed";
        return -1;
    }


    if (fd > STDERR_FILENO) {
        if (close(fd) == -1) {
//			tolog("close(fd) failed", LOG_ERROR, Ctx);
			perror("close(fd) failed");        		
            return -1;
        }
    }

    return pid;
}

void
signal_handler(int sig) {
	switch(sig) {
		
		case SIGHUP:
			ini_parse(&Ctx);
			tolog(&Ctx, LOG_ERROR, "server restarted ");
			evhttp_free(httpd);
			httpd = evhttp_start(Ctx.bind, Ctx.port);
			evhttp_set_gencb(httpd, http_handler, NULL);
			event_dispatch();

			break;
		case SIGTERM:
		case SIGINT:
			event_loopbreak();
			break;
        default:
            //syslog(LOG_WARNING, "Unhandled signal (%d) %s", strsignal(sig));
            break;
    }
}

void
setSignal() {
    signal(SIGHUP,  signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT,  signal_handler);
    signal(SIGQUIT, signal_handler);
}

void 
log_open(ctx * Ctx) {
	Ctx->log_fd = open(Ctx->logfile,O_WRONLY|O_CREAT|O_APPEND );
	if (Ctx->log_fd <1) {
		perror("can't open log file");
		exit(1);
	}   	

	fchmod(Ctx->log_fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (Ctx->log_fd <1) {
		perror("can't assign permission");
		exit(1);
	}   	
	
	Ctx->err_fd = open(Ctx->errfile,O_WRONLY|O_CREAT|O_APPEND );
	if (Ctx->err_fd <1) {
		perror("can't open log file");
		exit(1);
	}   	

	fchmod(Ctx->err_fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (Ctx->err_fd <1) {
		perror("can't assign permission");
		exit(1);
	}   	
	
	//printf("fd=%d", Ctx->log_fd);
}

#endif