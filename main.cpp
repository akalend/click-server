/*
 *  main.cpp
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

#include <signal.h>
#include <err.h>
#include <event.h>
#include <evhttp.h>

#include "clicker.h"
	
#include "conf.c"
#include "ini_parse.c"
#include "tools.c"
#include "http_handler.cpp"

#include "inputMcStorage.cpp"

ctx  Ctx;

struct evhttp *httpd;

int main( int argc, const char** argv ){

	struct event_base *ev_base; 
	int res=0;
	int is_stop=0;

	if (argc > 1) {
		Ctx.confile = (char *) argv[1];
		if (strcmp(Ctx.confile,"stop")==0) {
			is_stop=1;
			Ctx.confile = CLICKER_CONF;
		printf( "option stop\n");			
		}
	} else
		Ctx.confile = CLICKER_CONF;

	ini_parse(&Ctx);
	
	if (is_stop) {
		pid_t pid = pid_read(&Ctx); 
		if (pid > 0) {
			kill(pid, SIGTERM);
		}
		free_ctx(&Ctx);					
		return 0;
	}
	
	if (pid_exist(&Ctx)) {
		printf( "the pid file exist or daemon already started\n");
		free_ctx(&Ctx);		
		return 1;
	}
	
	if (Ctx.daemon) {
		res=demonize(&Ctx);
		if (res<1) {
			free_ctx(&Ctx);	
			if (res==-1) {
				return 1;
			}
			return 0;
		}
	}

	Ctx.buf = (char*)malloc(BUFF_SIZE);	
	if (!Ctx.buf) {
		perror( "can't allocate memory\n");
		free_ctx(&Ctx);		
		return 1;
	}

	IStorage * inStorage  = reinterpret_cast<IStorage *>(new inputMcStorage);
	if (!inStorage->init(&Ctx)) {
		perror( "can't initialize the in Storage\n");
		delete inStorage;
		free_ctx(&Ctx);	
		return 1;
	}

	IStorage * outStorage = reinterpret_cast<IStorage *>(new outputMongoStorage);
	if (!outStorage->init(&Ctx)) {
		perror( "can't initialize the out Storage\n");
		delete outStorage;
		delete inStorage;
		free_ctx(&Ctx);	
		return 1;
	}
		
	log_open(&Ctx);
	
	res=pid_create(&Ctx);
	if (res) {
		tolog(&Ctx, LOG_ERROR, "can not create pid file" );
		log_close(&Ctx);
		delete inStorage;
		free_ctx(&Ctx);	
		return 1;
	}
		
//printf("input port %d host:%s dbname=%s\n", Ctx.inDb.port, Ctx.inDb.host, Ctx.inDb.dbname);

	setSignal();
		
	tolog(&Ctx, LOG_NOTICE, "server started" );

    ev_base = event_init();
	httpd = evhttp_new (ev_base);
	
    //httpd = evhttp_start(Ctx.bind, Ctx.port);
	if (evhttp_bind_socket (httpd, Ctx.bind, (u_short)Ctx.port )) { 
		tolog(&Ctx, LOG_ERROR, "Failed to bind %s:%d", Ctx.bind, Ctx.port);
		log_close(&Ctx);
		delete outStorage;
		delete inStorage;			
		free_ctx(&Ctx);
		exit (1); 
	} 	
	
	res=0;
	if (!res) {
		evhttp_set_gencb(httpd, http_handler, NULL);

		event_base_dispatch(ev_base);
		evhttp_free(httpd);
	}

	/* free and close resorce*/
	pid_delete(&Ctx);
	tolog(&Ctx, LOG_NOTICE, "server finished ");

	log_close(&Ctx);
	delete outStorage;
	delete inStorage;	
	free_ctx(&Ctx);	

	return 0;
}
