/*
 *  ini_parse.cpp
 *  clicker-server
 *
 *  Created by Alexandre Kalendarev on 04.11.10
 *
 */


#ifndef __PARSEINI__
#define __PARSEINI__

#include "clicker.h"

int ini_parse( ctx * Ctx) {
	
	Scanner in;
    int t;
    memset((char*) &in, 0, sizeof(in));
    in.fd = 0;
	unsigned int len;
	
	in.fd = open( Ctx->confile, O_RDONLY);
	if (in.fd <1) {
	//	printf("conf %s\n", Ctx->confile);
		perror("can't open config file");
		exit(1);
	}   	
		
	bzero(Ctx,sizeof(Ctx));
	Ctx->port = 80;
	Ctx->log_level = 1;
	
    while((t = scan(&in)) != 0){
	
		len = in.cur-in.ptok-1;

//		printf("%d\t'%.*s'   line=%d  param=%d\n", t, in.cur - in.tok, in.tok ,in.line+1, in.parm);
//		printf("%d %d '%.*s' \n\n", in.cur - in.tok ,in.cur-in.ptok,len, in.ptok);
  
		switch ( in.parm ) {
			
			case 1: {
			if (t==PARSE_BOOL_ON)
				Ctx->daemon=1;
				break;
			} 	
			
			case 2: {
				if (t==PARSE_PATH) {
				 Ctx->logfile = (char*)malloc(len+1);
				 if (Ctx->logfile ==NULL) {
					perror("malloc logfile");
					return 1;
				}
					
				 memcpy((char*)Ctx->logfile,in.ptok,len);
				 *((char*)Ctx->logfile+len) = '\0';
				} else {
					printf("WARNING: log filename parse error\n");
					break;				
				}
				break;
			} 	
			
			case 3: {				
				if (t==PARSE_INT) {
				Ctx->port = atoi((const char *)in.ptok);
				}
				break;
			} 	
			
			case 4: {
				if (t==PARSE_INT) {
				Ctx->inDb.port = atoi((const char *)in.ptok);
				}
				break;
			} 	
			
			case 5: {			
				Ctx->log_level = t;
				if (Ctx->log_level<1) {
					printf("WARNING: log level parse error\n");
					Ctx->log_level=1;
				}	
				break;
			}
			
			case 6: {			
				if (t==PARSE_IP) {
				 Ctx->bind = (char*)malloc(len+1);
				 if (Ctx->bind == NULL) {
					perror("WARNING: malloc bind");
					return 1;
				}

				 memcpy((char*)Ctx->bind,in.ptok,len);
				 *((char*)Ctx->bind+len)='\0';
				} else {
					printf("WARNING: http IP parse error\n");
					break;				
				}
				break;
			}
			
			case 7: {
				if (t==PARSE_PATH) {
				 Ctx->pidfile = (char*)malloc(len+1);
				 if (Ctx->pidfile ==NULL) {
					perror("malloc pidfile");
					return 1;
				}
					
				 memcpy((char*)Ctx->pidfile,in.ptok,len);
				 *((char*)Ctx->pidfile+len)='\0';
				} else {
					printf("WARNING: pid filename parse error\n");
					break;				
				}
				break;
			} 	
			
			case 8: {			
				if (t==PARSE_IP) {
					 Ctx->inDb.host = (char*)malloc(len+1);
					 if (Ctx->inDb.host == NULL) {
						perror("WARNING: malloc DB.in host");
						return 1;
					}

					memcpy((char*)Ctx->inDb.host,in.ptok,len);
					*((char*)Ctx->inDb.host+len)='\0';
				} else {
					printf("WARNING: DB.in host parse error\n");
					break;				
				}
				break;
			}
			case 9: {
				if (t==PARSE_PATH) {
				 Ctx->inDb.dbname = (char*)malloc(len+1);
				 if (Ctx->inDb.dbname == NULL) {
					perror("malloc DB.in");
					return 1;
				}
					
				 memcpy((char*)Ctx->inDb.dbname,in.ptok,len);
				 *((char*)Ctx->inDb.dbname+len)='\0';
				} else {
					printf("WARNING: DB.in name parse error\n");
					break;				
				}
				break;
			} 	

			case 10: {
				if (t==PARSE_INT) {
				Ctx->outDb.port = atoi((const char *)in.ptok);
				}
				break;
			} 	

			case 11: {			
				if (t==PARSE_IP) {
					 Ctx->outDb.host = (char*)malloc(len+1);
					 if (Ctx->outDb.host == NULL) {
						perror("WARNING: malloc DB.out host");
						return 1;
					}

					memcpy((char*)Ctx->outDb.host,in.ptok,len);
					*((char*)Ctx->outDb.host+len)='\0';
				} else {
					printf("WARNING: DB.out host parse error\n");
					break;				
				}
				break;
			}
			
			case 12: {
				if (t==PARSE_PATH) {
				 Ctx->outDb.dbname = (char*)malloc(len+1);
				 if (Ctx->outDb.dbname == NULL) {
					perror("malloc DB.in");
					return 1;
				}
					
				 memcpy((char*)Ctx->outDb.dbname,in.ptok,len);
				 *((char*)Ctx->outDb.dbname+len)='\0';
				} else {
					printf("WARNING: DB.out name parse error\n");
					break;				
				}
				break;
			} 	
			
			case 13: {
				if (t==PARSE_PATH) {
				 Ctx->errfile = (char*)malloc(len+1);
				 if (Ctx->errfile == NULL) {
					perror("malloc errorlog");
					return 1;
				}
					
				 memcpy((char*)Ctx->errfile,in.ptok,len);
				 *((char*)Ctx->errfile+len)='\0';
				} else {
					printf("WARNING: errorlog parse error\n");
					break;				
				}
				break;
			} 	

			case 14: {
				if (t==PARSE_PATH) {
				 Ctx->prefix = (char*)malloc(len+1);
				 if (Ctx->prefix == NULL) {
					perror("malloc errorlog");
					return 1;
				}
					
				 memcpy((char*)Ctx->prefix,in.ptok,len);
				 *((char*)Ctx->prefix+len)='\0';
				} else {
					printf("WARNING: errorlog parse error\n");
					break;				
				}
				break;
			} 	

			case 15: {
				if (t==PARSE_PATH) {
				 Ctx->uri404 = (char*)malloc(len+1);
				 if (Ctx->uri404 == NULL) {
					perror("malloc uri404");
					return 1;
				}
					
				 memcpy((char*)Ctx->uri404,in.ptok,len);
				 *((char*)Ctx->uri404+len)='\0';
				} else {
					printf("WARNING: uri 404 parse error\n");
					break;				
				}
				break;
			} 	

			default : {
				printf("WARNING: parse error\n");
				break;
			}
		}
		in.parm = 0;
    }
	
    close(in.fd);
	
	if (!Ctx->logfile) {
		Ctx->logfile=strdup(CLICKER_LOG);
	}

	if (!Ctx->errfile) {
		Ctx->errfile=strdup(CLICKER_ERR);
	}

	if (!Ctx->pidfile) {
		Ctx->pidfile=strdup(CLICKER_PID);
	}
	
	if (!Ctx->bind) {
		printf("ERROR: http IP error\n");
		return 1;
	}

	if (!Ctx->inDb.host) {
		Ctx->inDb.host=strdup(CLICKER_LHOST);
	}

	if (!Ctx->outDb.host) {
		Ctx->outDb.host=strdup(CLICKER_LHOST);
	}
	
	//printf("daemon %d\n log level=%d '%s'\nport=%d %s\namqp '%s'\n", Ctx->daemon,Ctx->log_level, Ctx->logfile,Ctx->port,Ctx->bind,Ctx->amqp_connect);
	
	return 0;
}
#endif