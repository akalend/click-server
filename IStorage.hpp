/*
 *  IStorage.cpp
 *  click-server
 *
 *  Created by Alexandre Kalendarev on 05.11.10.
 *  Copyright 2010 
 *
 */

#ifndef __CLICKER_ISTORAGE__
#define __CLICKER_ISTORAGE__

/**
* the Interface of the Storage Engine
* The server have two storage:
* - input, where data (redirect url) get from storage by key 
*		(some parameter) and make redirect
* - output, when data (time, IP, key) loged to storage
*/
class IStorage {
	protected:
		int statusCode;
		
	public:

		/**
		* close storage engine, make disconnect
		*/
		virtual ~IStorage(){};
		
		/**
		* initial storage engine, make connect
		*/
		virtual int init(ctx * Ctx){};	
		
		/**
		* get data from storage by key into buffer buf 
		*/
		virtual void get(const char * key, char * buf){};	
		
		/**
		* store log data : time, IP, key
		* to storage 
		*/
		virtual void set(const char * key, int statusCode){};	
		
		/**
		* the result of the get/set
		* use status code:
		*	200 - Ok
		*	404 - not found, for get only
		*	500 - internal error
		*/
		virtual int getStatusCode(){
			return statusCode;
		};	

};

#endif
