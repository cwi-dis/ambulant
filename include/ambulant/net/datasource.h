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
	
	
	void show(bool verbose);									// show information about the buffer, if verbose is true the buffer is dumped to cout;
	void get_data(bytes *data, int size); 				//retrieve data from buffer,  still thinking about arguments.
																				//there should be something to pass on error messages.
	void put_data(bytes *data , int size);			 	// this one puts data alway at the end.						 
	
};

} //end namespace datasource

} // end namespace net

} //end namespace ambulant


#endif  //AMBULANT_NET_DATASOURCE_H
