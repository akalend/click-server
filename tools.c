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

#include "ini_parse.c"
#include "http_handler.cpp"

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
		err = 1;
	}
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


#endif