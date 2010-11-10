/*
 *  clicker.h
 *  clicker-server
 *
 *  Created by Alexandre Kalendarev on 04.11.10.
 *
 */

#ifndef __CLICKER_HEADER_SERVER__
#define __CLICKER_HEADER_SERVER__

//#define CLICKER_CONF	"/usr/local/etc/clicker.conf"
#define CLICKER_CONF	"clicker.conf"

#define CLICKER_LOG		"/var/log/clicker.log"
#define CLICKER_ERR		"/var/log/clicker.err.log"
#define CLICKER_PID		"/var/run/clicker.pid"

#define CLICKER_LHOST	"127.0.0.1"

#define BUFF_SIZE		1024

#include <memory>

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
#include <sys/types.h>
#include <sys/uio.h>
#include <cstdarg>
#include <evhttp.h>
#include <event.h>
#include <signal.h>
#include <err.h>

#define PARSE_IP		100
#define PARSE_INT		200

#define PARSE_BOOL_ON	101
#define PARSE_BOOL_OFF	102

#define PARSE_CONN_STR	301
#define PARSE_EXPR		302
#define PARSE_PATH		303

typedef struct {
    int			fd;
    unsigned char		*bot, *tok, *ptr, *cur, *pos, *lim, *top, *eof, *ptok,parm;
    unsigned int		line;
} Scanner;


enum log_type_e {
	LOG_STDERR,
	LOG_ERROR,
	LOG_CONTENT,
	LOG_ALERT,
	LOG_WARNING,
    LOG_NOTICE,
	LOG_DEBUG
};

typedef  struct {
	const char *	host;
	const char *	dbname;	
	void *			ctx;
	void *			theClass;	
	short			port;
	short			active;
} dbCtx;

typedef struct  {
	char			ip[16];
	int				log_fd;
	int				err_fd;
	int				pidfd;
	dbCtx			inDb;
	dbCtx			outDb;
	const char *	confile;
	const char *	bind;
	const char *	logfile; 	
	const char *	errfile; 		
	const char *	pidfile; 	
	const char *	prefix; 
	const char *	uri404; 			
	char **			ppa;
	char *			buf;
	short			daemon;
    short			port;
	short			log_level;
} ctx;

int scan(Scanner *s);

extern void tolog( ctx * Ctx,  int type, const char * fmt, ...);
#endif