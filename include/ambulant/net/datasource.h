/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */
 
#ifndef AMBULANT_NET_DATASOURCE_H
#define AMBULANT_NET_DATASOURCE_H

#ifndef WIN32

#include "ambulant/lib/callback.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"

// temporary debug messages
#include <iostream>
#include <ostream>
//#include <stdio.h>
//#include <stdlib.h>
//#include <iomanip>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


namespace ambulant {

namespace net {

typedef char bytes; 

class databuffer  
{
 private: 
     // duh, pointer to the buffer.
	char *m_buffer; 			
		     
	// the size of the bufer.
	int m_size;  			
	
	// how many bytes are in the buffer 														 
	int m_used;							
										
	// shift down all data above pos
	void shift_down(int pos);										
 public:
	// constructors
	databuffer();				
	databuffer(int size);	

	databuffer(databuffer& buf);    	  
	
	// destructor
	~databuffer();
	
	void resize(int newsize);
	
	// show information about the buffer, if verbose is true the buffer is dumped to cout;
	void dump(std::ostream& os, bool verbose) const;		
	
	//retrieve data from buffer,  still thinking about arguments.							
	void get_data(char *data, int size); 				

	// this one puts data alway at the end.															
	void put_data(char *data , int size);			 							 
	
	// returns the amount of bytes that are used.
	int used() const;
};

inline std::ostream& operator<<(std::ostream& os, const databuffer& n) {
	os << "databuffer(" << (void *)&n << ", used=" << n.used() << ")";
	return os;
}


// forward decleration
class active_datasource;




class passive_datasource : public ambulant::lib::ref_counted_obj
{
public:
	
	// constructor 
	passive_datasource(const std::string& url)
	:   m_url(url) {}
	
	// copy constructor
	//	passive_datasource(passive_datasource& ds);

	~passive_datasource();
	
	const std::string& get_url() const { return m_url; }
	active_datasource *activate();
	
	friend inline std::ostream& operator<<(std::ostream& os, const passive_datasource& n) {
		os << "passive_datasource(" << (void *)&n << ", url=\"" << n.m_url << "\")";
		return os;
	}
	
private:
	const std::string m_url;
};

	
 
class active_datasource : public ambulant::lib::ref_counted_obj {  	
public:
	// constructors 
	active_datasource();
	active_datasource(passive_datasource *const source, int file);
	 // destructor
	~active_datasource();


	void start(ambulant::lib::event_processor *evp,ambulant::lib::event *readdone);


	//  Get data from buffer and put 'size' bytes in buffer.
	void read(char *data, int size);

	// Return the amount of data currently in buffer.
	int size() const;
	
	friend inline std::ostream& operator<<(std::ostream& os, const active_datasource& n) {
		os << "active_datasource(" << (void *)&n << ", source=" << (void *)n.m_source << ")";
		return os;
	}

private:
    databuffer *m_buffer;
    passive_datasource *m_source;
	int m_filesize;
	int m_stream;
	void filesize();
    void read_file();

};

} // end namespace net

} //end namespace ambulant


#else // WIN32 is defined

///////////////////////////////////////////////
// WARNING: 
// Though the implementation in this section is std c++
// its only purpose is for testing under win32.
// This implementation violates some parts of the assumed protocol.
// For example:
// active_datasource::read() when asked to fill
// a buffer it returns a pointer to the data.
// Renderer will call the c-function ::free against 
// this pointer. So, ...


#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/event_processor.h"

#include <string>

// debug
#include "ambulant/lib/logger.h"
#include <sstream>
#include <fstream>

namespace ambulant {

namespace net {

typedef  std::basic_string<byte> databuffer;

class active_datasource;

using ambulant::lib::logger;

class passive_datasource : public ambulant::lib::ref_counted_obj {

  public:
	passive_datasource(const char *url)
	:	m_url(url?url:"") {
		logger::get_logger()->debug(repr() + "::cstr()");
	}
	
	passive_datasource(const std::string& url)
	:	 m_url(url) {
		logger::get_logger()->debug(repr() + "::cstr()");
	}
	
	~passive_datasource() {
		logger::get_logger()->debug(repr() + "::dstr()");
	}
	
	active_datasource *activate();
	
	std::string repr() {
		std::ostringstream os;
		os << "passive_datasource(" << m_url << ")";
		return os.str();
	};
	
	const std::string& get_url() const { return m_url;}
	
  private:
	std::string m_url;
};

class active_datasource : public ambulant::lib::ref_counted_obj {
  public:
	typedef std::basic_string<byte> buffer_type;
	typedef buffer_type::value_type value_type;
	typedef buffer_type::pointer pointer;
	typedef buffer_type::const_pointer const_pointer;
	typedef buffer_type::size_type size_type;

	active_datasource() 
	:	m_source(0) {
		logger::get_logger()->debug(repr() + "::cstr()");
	}
	
	active_datasource(passive_datasource *source) 
	:	m_source(source) {
		logger::get_logger()->debug(repr() + "::cstr()");
	}
	
	~active_datasource() {
		logger::get_logger()->debug(repr() + "::dstr()");
	}
	
	void start(ambulant::lib::event_processor *evp, ambulant::lib::event *readdone) {
		logger::get_logger()->trace(repr() + "::start()");
		std::ifstream ifs(m_source->get_url().c_str());
		if(!ifs) {
			logger::get_logger()->error(repr() + "::read() failed");
			return;
		}
		const size_t buf_size = 1024;
		char *buf = new char[buf_size];
		while(!ifs.eof() && ifs.good()){
			ifs.read(buf, buf_size);
			m_buffer.append((byte*)buf, ifs.gcount());
		}
		delete[] buf;
		evp->add_event(readdone, 0, ambulant::lib::event_processor::high);
	}

	void read(char *data, int size) {
		logger::get_logger()->trace(repr() + "::read()");
		data = (char*)m_buffer.data();
	}

	size_type size() const { return m_buffer.size();}
	
	std::string repr() {
		std::ostringstream os;
		os << "active_datasource[" << (m_source?m_source->repr():"NULL") + "]";
		return os.str();
	};
	
	friend inline std::ostream& operator<<(std::ostream& os, const active_datasource& n) {
		os << "active_datasource(" << (void *)&n << ", source=" << (void *)n.m_source << ")";
		return os;
	}
  private:
	passive_datasource *m_source;
	databuffer m_buffer;
  
};

//////////////////
// Inline implemetation

inline active_datasource *passive_datasource::activate() { 
	ambulant::lib::logger::get_logger()->trace(repr() + "::activate()");
	return new active_datasource(this);
}

} // end namespace net

} //end namespace ambulant

#endif // end of (ifndef WIN32 / else / endif) block.

///////////////////////////////////////////////

#endif  //AMBULANT_NET_DATASOURCE_H
