/*
 *  http_handler.cpp
 *  amqp-rest
 *
 *  Created by Alexandre Kalendarev on 01.05.10.
 *
 */

#ifndef __HTTPHANDLER__
#define __HTTPHANDLER__

#include <err.h>
#include <event.h>
#include <evhttp.h>
#include <memory>
#include <cstdarg>

#include "tools.c"
#include "inputMcStorage.cpp"
#include "outputMongoStorage.cpp"

void  tolog(ctx * Ctx, int type, const char * message , ...);
void  toAccessLog(ctx * Ctx, const char * message , ...);


extern ctx Ctx;

void 
parse_uri(const char * uri, char * name, int name_len, char * parm, int parm_len) {
	char * p = (char *)uri;
	char * pn = name;
	int len=0;

	while(*p && *p != '?' && len < name_len) {
		if (*p=='/') {			
			pn=name;
			bzero(pn,name_len);
			p++;
		}	
		*(pn++) =*(p++);
		len++;	
	}
	
	if (!*p)
		return;
	
	len=0;
	p++;
	pn=parm;	
	while(*p && len < parm_len) {			
		*(pn++) =*(p++);
		len++;	
	}		
	
}

void
http_request_stats(const char* key, struct evhttp_request *req, struct evbuffer *buf) {

	IStorage * pInStorage =  reinterpret_cast<IStorage *>(Ctx.inDb.theClass);
	
	if (!Ctx.inDb.theClass) {
		tolog(&Ctx, LOG_ERROR, "input storage class init error");
//		evbuffer_add_printf(buf, "Storage err\r\n");
		evhttp_send_reply(req, HTTP_SERVUNAVAIL, "internal error", buf);	
		evbuffer_free(buf);
		return;			
	}
	
	*Ctx.buf='\0';

	tolog(&Ctx, LOG_NOTICE, "uri='%s'",key);
	pInStorage->get(key,Ctx.buf);

//	evbuffer_add_printf(buf, Ctx.buf);
	int status = pInStorage->getStatusCode();
	toAccessLog(&Ctx, " %s GET\t%s\t%d", *Ctx.ppa, evhttp_request_uri(req), status);	


	IStorage * pOutStorage =  reinterpret_cast<IStorage *>(Ctx.outDb.theClass);
	pOutStorage->set(key,status);

	evhttp_add_header(req->output_headers, "Server", "clicker-server");
	
	if (status == 200) {
		evhttp_add_header(req->output_headers, "Location", Ctx.buf);
		evhttp_send_reply(req, HTTP_OK, "OK"	, buf);	
	}	

	if (status == 404) {
//		evhttp_add_header(req->output_headers, "Location", Ctx.buf);
		if (Ctx.uri404) {
			evhttp_add_header(req->output_headers, "Location", Ctx.uri404);
			evhttp_send_reply(req, HTTP_OK, "OK", buf);				
		} else {
			evhttp_send_reply(req, HTTP_NOTFOUND, "Not found", buf);	
		}	
	}	

	if (status == 500) {
//		evhttp_add_header(req->output_headers, "Location", Ctx.buf);
		evhttp_send_reply(req, HTTP_SERVUNAVAIL, "internal error", buf);
	}	

	
	evbuffer_free(buf);
	return;

}

void 
http_handler(struct evhttp_request *req, void *arg) {
		
	const char * uri = evhttp_request_uri(req);
	char name[256]; bzero(name,256);
	char parm[16];  bzero(parm,16);
	
	parse_uri(uri, name, 256, parm,16);

//	tolog(&Ctx, LOG_NOTICE, "name='%s' param='%s'",name,parm);

	u_short port=0;	
	bzero(Ctx.ip,16);
	char * pa=Ctx.ip;
	char ** ppa=&pa;
	evhttp_connection_get_peer(req->evcon, ppa, &port);
	Ctx.ppa=ppa;

	struct evbuffer *buf;			
	buf = evbuffer_new();
	if (!buf) {
		tolog( &Ctx, LOG_STDERR, "failed to create response buffer");
		return;
	}
	
	if (Ctx.prefix && !strcmp(Ctx.prefix,name)) {
		http_request_stats(parm, req, buf);
		return;
	}
	
	if (strlen(uri)>1)
		http_request_stats((uri+1), req, buf);
	///????
	return;

}


#endif