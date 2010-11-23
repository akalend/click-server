/*
 *  server.cpp
 *  click-server  
 *
 *  Created by Alexandre Kalendarev on 09.11.10.
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
 *
 */

#include "clicker.h"
#include "ini_parse.c"

#include "tools.c"
#include "conf.c"

#include "http_handler.cpp"

#include "IStorage.hpp"

#include "inputMcStorage.cpp"
#include "outputMongoStorage.cpp"

struct evhttp *httpd;

class ClkServer
{
	private:
		ctx Ctx;
		bool needStop;
		bool isLogOpen;
		IStorage * inStorage;
		IStorage * outStorage;

	public:
		ClkServer() {
			needStop = false;
			isLogOpen = false;
			inStorage = NULL;
			outStorage = NULL;
			bzero((void*)&Ctx, sizeof(ctx));
		}	
		
		~ClkServer() {
		
			if (Ctx.pidfile) {
				free((void*)Ctx.pidfile);
			}

			if (Ctx.bind) {
				free((void*)Ctx.bind);
			}

			if (Ctx.inDb.host) {
				free((void*)Ctx.inDb.host);
			}

			if (Ctx.inDb.dbname) {
				free((void*)Ctx.inDb.dbname);
			}

			if (Ctx.outDb.host) {
				free((void*)Ctx.outDb.host);
			}

			if (Ctx.outDb.dbname) {
				free((void*)Ctx.outDb.dbname);
			}

			if (Ctx.buf) {
				free((void*)Ctx.buf);
			}

			if (Ctx.errfile) {
				free((void*)Ctx.errfile);
			}

			if (Ctx.prefix) {
				free((void*)Ctx.prefix);
			}

			if (Ctx.uri404) {
				free((void*)Ctx.uri404);
			}

			if (inStorage)
				delete inStorage;
	
			if (outStorage)
				delete outStorage;

			if (isLogOpen)	{
				Ctx.log_fd && close(Ctx.log_fd);
				Ctx.err_fd && close(Ctx.err_fd);
			}
		}
		
		void setConfig(const char * configName) {
			Ctx.confile = configName;
		}
		
		void setStop() {
			needStop = true;
		}

		void parseIni() {
			ini_parse(&Ctx);
		}

		bool checkStop() {
			if (!needStop)
				return false;
				
			pid_t pid = pidRead(); // pid_read(&Ctx); 
			if (pid > 0) {
				kill(pid, SIGTERM);
			}
			
			return true;
		}

		bool checkPid() {
			struct stat st;
			if (!stat(Ctx.pidfile,&st)) {
				return true;		
			}
			
			if (errno==ENOENT)
				return false;
				
			return true;				
		}
		
		void pidDelete() {
			flock(Ctx.pidfd, LOCK_UN);
			if ( close(Ctx.pidfd) < 0){
				tolog(&Ctx, LOG_ERROR, "can't close pid file %s", strerror(errno));
			//		perror("can't close pid file");
			}

			size_t res = remove(Ctx.pidfile);
			if (res != 0) {
				tolog(&Ctx, LOG_ERROR, "can't remove pid file %d" , strerror(errno));
			}

			tolog(&Ctx, LOG_NOTICE, "server finished ");
		}

		int demonize() {
			if (!Ctx.daemon)
				return 1;
		
			int  fd;
			
			switch (fork()) {
			case -1:
				perror("demonize fork failed");
				return -1;

			case 0:
				break;

			default:
				//free_ctx(Ctx);
				return 0;
			}

			int pid = getpid();

			if (setsid() == -1) {
				perror("setsid() failed");        
				return -1;
			}

			umask(0);

			for (int i = 0; i < 1024; i++)
				close(i);

			fd = open("/dev/null", O_RDWR);
			if (fd == -1) {
				perror("open(\"/dev/null\") failed");        
				return -1;
			}

			if (dup2(fd, STDIN_FILENO) == -1) {
				perror("dup2(STDIN) failed");        
				return -1;
			}

			if (dup2(fd, STDOUT_FILENO) == -1) {
				perror("dup2(STDOUT) failed");        		
				return -1;
			}

			if (dup2(fd, STDERR_FILENO) == -1) {
				perror("dup2(STDERR) failed");
				return -1;
			}


			if (fd > STDERR_FILENO) {
				if (close(fd) == -1) {
					perror("close(fd) failed");        		
					return -1;
				}
			}

			return pid;		
		}
		
		bool init() {
			Ctx.buf = (char*)malloc(BUFF_SIZE);	
			if (!Ctx.buf) {
				perror( "can't allocate memory\n");
				return false;
			}

			inStorage  = reinterpret_cast<IStorage *>(new inputMcStorage);
			if (!inStorage->init(&Ctx)) {
				perror( "can't initialize the in Storage\n");
				return false;
			}

			outStorage = reinterpret_cast<IStorage *>(new outputMongoStorage);
			if (!outStorage->init(&Ctx)) {
				perror( "can't initialize the out Storage\n");
				return false;
			}
			
			
			return true;
	}
	
	bool openLog() {
	
		Ctx.log_fd = open(Ctx.logfile,O_WRONLY|O_CREAT|O_APPEND );
		if (Ctx.log_fd <1) {
			perror("can't open log file");
			return false;
		}   	

		fchmod(Ctx.log_fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		if (Ctx.log_fd <1) {
			perror("can't assign permission");
			return false;
		}   	
		
		Ctx.err_fd = open(Ctx.errfile,O_WRONLY|O_CREAT|O_APPEND );
		if (Ctx.err_fd <1) {
			perror("can't open log file");
			return false;
		}   	

		fchmod(Ctx.err_fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		if (Ctx.err_fd <1) {
			perror("can't assign permission");
			return false;
		}   	
		
		tolog(&Ctx, LOG_NOTICE, "Server started");		
		isLogOpen = true;
		return true;
	}
	
	bool bindSocket() {
	
		struct event_base *ev_base;
		ev_base = event_init();
		httpd = evhttp_new (ev_base);
		
		//httpd = evhttp_start(Ctx.bind, Ctx.port);
		if (evhttp_bind_socket (httpd, Ctx.bind, (u_short)Ctx.port )) { 
			tolog(&Ctx, LOG_ERROR, "Failed to bind %s:%d", Ctx.bind, Ctx.port);
			return false;
		} 	
	
		evhttp_set_gencb(httpd, http_handler, &Ctx);

		event_base_dispatch(ev_base);
		evhttp_free(httpd);

		tolog(&Ctx, LOG_NOTICE, "server finished" );
		
		return true;
	}
	
	bool pidCreate() {
		Ctx.pidfd = open(Ctx.pidfile, O_CREAT | O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH );
		if (!Ctx.pidfd) {
			tolog(&Ctx, LOG_ERROR,  "can't open pid file %s", strerror(errno));
			return false;
		}

		// проверить блокировку
		flock(Ctx.pidfd, LOCK_EX | LOCK_NB);

		char buf[16]; 
		bzero(buf,16);
		sprintf(buf,"%ld",(long)getpid());	
		if (write( Ctx.pidfd,buf, strlen(buf)+1) < 1) {
			close(Ctx.pidfd);
			tolog(&Ctx, LOG_ERROR, "can't write to pid file %s" , strerror(errno));
		//		perror("can't write to pid file");
			return false;
		} 

		return true;	
	}
	
	void setSignal() {
		signal(SIGHUP,  signal_handler);
		signal(SIGTERM, signal_handler);
		signal(SIGINT,  signal_handler);
		signal(SIGQUIT, signal_handler);
		signal(SIGPIPE, SIG_IGN);
	}
	
	private:
	 int pidRead() {
		FILE * fd = fopen(Ctx.pidfile, "r" );
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
};
