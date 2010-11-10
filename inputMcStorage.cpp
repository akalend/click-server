/*
 *  inputMcStorage.cpp
 *  click-server
 *
 *  Created by Alexandre Kalendarev on 05.11.10.
 *  Copyright 2010 
 *
 */

#ifndef __CLICKER_INPUT_MCSTORAGE__
#define __CLICKER_INPUT_MCSTORAGE__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>
#include <arpa/inet.h>

#include "clicker.h"
#include "IStorage.hpp"


class inputMcStorage : IStorage
{
	private:
		int fd;
		int ls;
		ctx * _Ctx;
		int rc;
		
	public:	
	
	~inputMcStorage() {
		if (ls)
			close(ls);
		if (rc)
			close(rc);	
	}
	
	int init(ctx * Ctx)	 {
		ls=0;
		rc=0;
		
		Ctx->inDb.theClass = this;
		_Ctx = Ctx;
		struct hostent *he;
    	struct sockaddr_in addr; 
		int err,len;

		if(-1 == (ls = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) )) {
			perror("Socket can not created");
			return 0; /* false result*/
		}

		setsockopt( ls, SOL_SOCKET, SO_REUSEADDR, &rc, sizeof(rc) );
		memset( &addr , 0, sizeof(addr) );
		
		addr.sin_family = AF_INET;
		addr.sin_port = htons (Ctx->inDb.port);

		inet_aton(Ctx->inDb.host, &addr.sin_addr);				
		memset(addr.sin_zero, '\0', sizeof addr.sin_zero);

		if (connect(ls, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
			perror("connect() failed");
			return 0; /* false result*/
		}

		return 1;
	}

	void get(const char * key, char * buff) {
		size_t r=0;
		char * p=buff;
		statusCode = 0;		
		
		r = write(ls,"get ",4);
		if (r==-1) {
			statusCode = 500;
			tolog(_Ctx, LOG_ERROR, "can't write to socket %s",strerror(errno));
			strcpy(buff,"write err\0");		
			return;	
		}

		r=write(ls,key,strlen(key));
		if (r==-1) {
			statusCode = 500;
			tolog(_Ctx, LOG_ERROR, "can't write to socket %s",strerror(errno));
			strcpy(buff,"write err\0");		
			return;	
		}

		r=write(ls,"\n",1);
		if (r==-1) {
			statusCode = 500;
			tolog(_Ctx, LOG_ERROR, "can't write to socket %s",strerror(errno));
			strcpy(buff,"write err\0");		
			return;	
		}
		
		r = read(ls,p,5);
		p[5]='\0';

		if (strncmp(p,"END",3) == 0) {
			statusCode = 404;
			return;
		}

		if (strncmp(p,"VALUE",5) != 0) {
			statusCode = 500;
			tolog(_Ctx, LOG_ERROR, "protocol error, 'VALUE' not found");
			return;
		}
		
		p +=4;
		int n=0;
		while (1==read(ls,p,1)) {
			if(*p==' ')
				++n;
			if (n==3)
				break;
			
			if(*p=='\n') {
				n=0;
				break;	
			}				
			++p;	
		} 
		
		if (!n) {
			statusCode = 500;
			tolog(_Ctx, LOG_ERROR, "protocol error, 3 times space must be");
			return;		
		}
		
		p=buff;
		n=0;
		while (1==read(ls,p,1)) {
			if(*p=='\n') {
				break;	
			}	
			++n;			
			++p;	
		}
		p[n]='\0';		
		int len=atoi(buff);
		
		if (!len) {
			tolog(_Ctx, LOG_ERROR, "protocol error, strlen of data lenght is zero");
			statusCode = 500;
			return;		
		}

		p=buff;
		n=read(ls,p,len+2);
		if (!n) {
			tolog(_Ctx, LOG_ERROR, "can't read from socket %s",strerror(errno));
			statusCode = 500;
			return;		
		}
		
		buff[n-2]='\0';

		p = buff+len+1;
		n=0;
		while (1==read(ls,p,1)) {
			if(*p=='\n') {
				break;					
			}				
			++p;	
			++n;
		}
		p = buff+len+1;
		if(strncmp("END", p,3)) {
			tolog(_Ctx, LOG_ERROR, "protocol error, the 'END' not found");
			statusCode = 500;
			return;				
		}		
		
		/* 
		the HTTP statut code must be 200
		*/
		statusCode = 200;		
		return;		
	}

	/**
	* for input storage set not implemented
	*/
	void set(const char * key, int status){}	
};

#endif
