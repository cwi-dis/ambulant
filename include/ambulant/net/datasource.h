/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */
 
#ifndef AMBULANT_NET_DATASOURCE_H
#define AMBULANT_NET_DATASOURCE_H

#include "ambulant/lib/callback.h"
#include "url.h"

// temporary debug messages
#include <iostream>

namespace ambulant {

namespace net {

namespace datasource {
	

class data_buffer {
	
	private:
	 // bytes moet een int of long int of zo worden ?
	 // ik wil hier eigenlijk niet nadenken over wat voor een data het is
	 
	bytes *buffer; // duh, pointer naar de buffer.
	 public:
	// constructors
	data_buffer()
	data_buffer(bytes size)
	
	// destructor
	~data_buffer()
	
	increase_buffer(bytes by)
	decrease_buffer(bytes by)
	
	p_bytes get_data( ... )  // Haal data uit buffer, argumenten later bedenken.
	put_data(p_bytes data, int pos ) // Zet data in buffer, pos geeft aan waar, begin of eind.
	
}

class passive_datasource {
	
}

class active_datasource {
	
}


	
	
} //end namespace datasource

} // end namespace net

} //end namespace ambulant