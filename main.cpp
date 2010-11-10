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
#include "clicker.h"
#include "clkServer.cpp"	

int main( int argc, const char** argv ){

	if (argc == 1) {
		printf( "Usage:\n");			
		printf( "\t%s [--start | --stop | /path/to/config/file.conf ]\n",argv[0]);			
		printf( "\toption --start - start server with default path to config (%s)\n",CLICKER_CONF);			
		printf( "\toption --stop - stop server, using default path to config\n");			
		printf( "\t/path/to/config/%s.conf - start server with any config file\n",argv[0]);			
		return 0;
	}

//	ClkServer s;
	auto_ptr<ClkServer> server(new ClkServer());

	if (argc > 1) {
		if (strcmp(argv[1],"--stop")==0) {
			server->setConfig(CLICKER_CONF);
			server->setStop();
		} else if (strcmp(argv[1],"--start")==0) {
			server->setConfig(CLICKER_CONF);
		} else {
			server->setConfig(argv[1]);
		}		
	}
	
	server->parseIni();
	if (server->checkStop()) {
		printf( "option stop\n");			
		return 0;
	}

	if (server->checkPid()) {
		printf( "the pid file exist or daemon already started\n");
		return 1;
	}
	int res;
	if ((res = server->demonize()) < 1) {
		if (res == -1)
			return 1;
		return 0;
	}

	if (!server->init()) {
		return 1;
	}
				
	server->openLog();

	if (!server->pidCreate())
		return 1;

	server->setSignal();

	if (!server->bindSocket()) {
		return 1;
	}
	server->pidDelete();

	return 0;
}
