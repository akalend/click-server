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

#include <libmemcached/memcached.h>

#include "clicker.h"
#include "tools.c"
#include "inputStorage.cpp"

class inputMcStorage :  inputStorage
{
	private:
		memcached_st * memc;
		memcached_return rc;
		ctx * _Ctx;
	public:	
	
	~inputMcStorage() {
		memcached_deinit(memc);
	}
	
	int init(ctx * Ctx)	 {
		_Ctx = Ctx;
		memc = memcached_init(NULL);
		Ctx->inDb.theClass = reinterpret_cast<void *>(this);
		rc= memcached_server_add(memc, (char *)Ctx->inDb.host, (unsigned int)Ctx->inDb.port);
//		return 1;
		return (rc == MEMCACHED_SUCCESS);
	}

	void get(const char * key, char * buff) {

//		memcached_st * memc1 = memcached_init(NULL);
//		rc= memcached_server_add(memc1, "localhost", 0);
		
		size_t length=0;
		uint16_t flags=0;
		memcached_return error = (memcached_return)0;
		char *outdata = memcached_get(memc, (char*)key, strlen(key),
								&length, &flags, &error);

		if (error) {
			strcpy(buff,"mc error\0");		
		} else {
			strncpy(buff,outdata,length);
		}
			
		if (outdata)
			free(outdata);
						
//		memcached_deinit(memc1);
	}
	
};

#endif
