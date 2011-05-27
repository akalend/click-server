/*
 *  outputMongoStorage.cpp
 *  
 *
 *  Created by Alexandre Kalendarev on 07.11.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __CLICKER_OUTPUT_MONGO_STORAGE__
#define __CLICKER_OUTPUT_MONGO_STORAGE__

void 
tolog( ctx * Ctx,  int type, const char * fmt, ...);

#include "clicker.h"
#include "tools.c"

#include "IStorage.hpp"
#include <mongo/client/dbclient.h>
#include <mongo/client/model.h>

using namespace std;
using namespace mongo;

class outputMongoStorage : IStorage
{
	private:

		ctx * _Ctx;
		DBClientConnection * conn;
		
	public:	
	
	~outputMongoStorage() {
		if (conn)
			delete conn;
	}
	
	int init(ctx * Ctx)	 {
		
		Ctx->outDb.theClass = this;
		_Ctx = Ctx;
		
		conn = new DBClientConnection;
		string errmsg;	
		char buf[32];
		bzero(buf,32);

		if (Ctx->outDb.port) {
			sprintf(buf, "%s:%d", Ctx->outDb.host,Ctx->outDb.port);
		} else {
			sprintf(buf, "%s", Ctx->outDb.host);
		}

		string connectionStr = buf;
				
		if (!conn->connect(connectionStr, errmsg)) {
			string msg("mongo connect failed:"); 
			printf( (msg + errmsg).c_str() );
			return 0; /* false result*/
		}

		return 1;
	}

	/**
	* for output storage get not implemented
	*/
	void get(const char * key, char * buff) {}

	/**
	*/
	void set(const char * key, int statusCode){
	
		string collection= _Ctx->outDb.dbname + string(".leads");
		
		BSONObjBuilder b;

		const time_t log_time = time(0);
		b.append("time", (int)log_time);
		b.append("key", key);
		b.append("IP", *(_Ctx->ppa));
		b.append("status", statusCode);
				
		conn->insert(collection, b.obj());
	}	
};

#endif


