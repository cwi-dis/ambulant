/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */
 
#ifndef AMBULANT_NET_DATASOURCE_H
#define AMBULANT_NET_DATASOURCE_H

//#include "ambulant/lib/callback.h"


// temporary debug messages
#include <iostream>
#include <ostream>
#include <iomanip>
#include <cstring>


namespace ambulant {

namespace net {

namespace datasource {
	
typedef char bytes; 
typedef char url;


class databuffer 
{
 private: 
	bytes *buffer; 				     // duh, pointer to the buffer.
	int s;  																	 // the size of the bufer.
	int used;																// howmany bytes are in the buffer 
	
	void shift_down(int pos);										// shift down all data above pos.

 public:
	// constructors
	databuffer();				    // default constructor
	databuffer(int size);			    // natural constructor

	databuffer(databuffer& buf);    	    // copy constructor
	
	// destructor
	~databuffer();
	
	void resize(int newsize);
	void show(bool verbose);									// show information about the buffer, if verbose is true the buffer is dumped to cout;
	void get_data(bytes *data, int size); 				//retrieve data from buffer,  still thinking about arguments.
																				//there should be something to pass on error messages.
	void put_data(bytes *data , int size);			 	// this one puts data alway at the end.						 
	
};

class passive_datasource
{
private:
	url where;
	
public:
	passive_datasource();
	passive_datasource(url location);
	passive_datasource(passive_datasource& ds);
	
	~passive_datasource();
	
	void activate();
};


class active_datasource
{
private:
	databuffer buffer();
	
public:

void start_read_all(url loc);


};

} //end namespace datasource

} // end namespace net

} //end namespace ambulant


#endif  //AMBULANT_NET_DATASOURCE_H
