/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

 
#ifndef AMBULANT_NET_DATASOURCE_H
#define AMBULANT_NET_DATASOURCE_H

#include "ambulant/config/config.h"

#ifndef AMBULANT_PLATFORM_WIN32

#include "ambulant/lib/callback.h"
#include "ambulant/lib/refcount.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/event_processor.h"

// temporary debug messages
#include <iostream>
#ifndef AMBULANT_NO_OSTREAM
#include <ostream>
#else /*AMBULANT_NO_OSTREAM*/
#include <ostream.h>
#endif/*AMBULANT_NO_OSTREAM*/
//#include <stdio.h>
//#include <stdlib.h>
//#include <iomanip>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef AMBULANT_PLATFORM_WIN32_WCE
#include <unistd.h>
#endif


namespace ambulant {

namespace net {

typedef char bytes; 

class databuffer  
{
 private: 
    
	char* m_buffer; 			
	unsigned long int m_rear;
	
	unsigned long int m_size;  			
 	unsigned long int m_max_size; 														 
	unsigned long int m_used;							
	
	bool m_buffer_full;
							
 public:
	// constructors
	databuffer();				
	databuffer(int max_size);	
	
	// destructor
	~databuffer();

	
	// show information about the buffer, if verbose is true the buffer is dumped to cout;
	void dump(std::ostream& os, bool verbose) const;		
	
								
	//void get_data(char *data, int size); 				

	// this one puts data alway at the end.															
	//void put_data(char *data , int size);			 							 
	
	// returns the amount of bytes that are used.
	int used() const;
 	bool is_full();
    bool not_empty();
	void readdone(int size);
	char * prepare();
	void pushdata(int size);
    char* get_read_ptr();
	
};

inline std::ostream& operator<<(std::ostream& os, const databuffer& n) {
	os << "databuffer(" << (void *)&n << ", used=" << n.used() << ")";
	return os;
}
 
class abstract_active_datasource : public ambulant::lib::ref_counted_obj {  	
  public:
	~abstract_active_datasource() {}

	virtual void start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback) = 0;  

        // a readdone cal is made by the client if he is ready with len bytes of data.
    virtual void readdone(int len) = 0;
//    virtual void callback() = 0 ;
    virtual bool end_of_file() = 0;
	virtual bool buffer_full() = 0;
		
	virtual char* read_ptr() = 0;
	virtual int size() const = 0 ;
	
};


class abstract_audio_datasource : virtual public abstract_active_datasource {
  public:
	  ~abstract_audio_datasource() {};
		  
    virtual void start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback) = 0;  

        // a readdone cal is made by the client if he is ready with len bytes of data.
    virtual void readdone(int len) = 0;
    virtual bool end_of_file() = 0;
	virtual bool buffer_full() = 0;
		
	virtual char* read_ptr() = 0;
	virtual int size() const = 0;
	
	virtual int get_nchannels () = 0;
  	virtual int get_nbits () = 0;
	virtual int get_samplerate () = 0;
	virtual int select_decoder(char* file_ext) = 0;
  protected:
    virtual void callback() = 0 ;
 
};


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

	


class active_datasource : virtual public abstract_active_datasource {
  public:
	active_datasource();
	active_datasource(passive_datasource *const source, int file);
  	~active_datasource();
  	
  	void start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback);
	void readdone(int len);
    void callback();
    bool end_of_file();
	bool buffer_full();
		
	char* read_ptr();
	int size() const;

  
	void read(char *data, int size);
  	
  	friend inline std::ostream& operator<<(std::ostream& os, const active_datasource& n) {
		os << "active_datasource(" << (void *)&n << ", source=" << (void *)n.m_source << ")";
		return os;
	}
  private: 
	void filesize();
    void read_file();
	databuffer *m_buffer;
  	passive_datasource *m_source;
	int m_filesize;
	int m_stream;
	bool m_end_of_file;
};

// This is a temporary class: it allows you to read raw audio files as
// 16bit mono 44k1 samples
class raw_audio_datasource : public abstract_audio_datasource {
  public:
  	raw_audio_datasource()
  	:	m_src(new active_datasource()) {}
  	
  	raw_audio_datasource(active_datasource *source)
  	:	m_src(source) { m_src->add_ref(); }
  	
  	raw_audio_datasource(passive_datasource *const source, int file)
  	:	m_src(new active_datasource(source, file)) {}
  	
  	~raw_audio_datasource() {
  		m_src->release();
  	}
  	
  	void start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback) {
  		m_src->start(evp, callback);
  	}
	void readdone(int len) { m_src->readdone(len); }
    void callback()  {}
    bool end_of_file() { return m_src->end_of_file(); }
	bool buffer_full() { return m_src->buffer_full(); }
		
	char* read_ptr() { return m_src->read_ptr(); }
	int size() const { return m_src->size(); }

	int get_nchannels () { return 1; }
  	int get_nbits () { return 16; }
	int get_samplerate () { return 44100; }
	int select_decoder(char* file_ext) {}
  private:
  	active_datasource *m_src;
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

#ifndef AMBULANT_NO_IOSTREAMS
#include <sstream>
#include <fstream>
#endif

namespace ambulant {

namespace net {

typedef  std::basic_string<byte> databuffer;

class active_datasource;

using ambulant::lib::logger;

class passive_datasource : public ambulant::lib::ref_counted_obj {

  public:
	passive_datasource(const char *url)
	:	m_url(url?url:"") {
		//logger::get_logger()->debug(repr() + "::cstr()");
	}
	
	passive_datasource(const std::string& url)
	:	 m_url(url) {
		//logger::get_logger()->debug(repr() + "::cstr()");
	}
	
	~passive_datasource() {
		//logger::get_logger()->debug(repr() + "::dstr()");
	}
	
	active_datasource *activate();
	
#if !defined(AMBULANT_NO_IOSTREAMS) && !defined(AMBULANT_NO_STRINGSTREAM)
	std::string repr() {
		std::ostringstream os;
		os << "passive_datasource(" << m_url << ")";
		return os.str();
	};
#endif
	
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
	:	m_source(0), m_gptr(0) {
	}
	
	active_datasource(passive_datasource *source) 
	:	m_source(source), m_gptr(0) {
	}
	
	~active_datasource() {
	}
	
	bool exists() const { 

#ifndef AMBULANT_NO_IOSTREAMS
		std::ifstream ifs(m_source->get_url().c_str());
		bool exists = ifs && ifs.good();
		ifs.close();
		return exists;
#else
		return false;
#endif

	}
	
	databuffer& get_databuffer() { return m_buffer;}
	
	const std::string& get_url() const { return m_source->get_url();}
	
	void start(ambulant::lib::event_processor *evp, ambulant::lib::event *readdone) {
#ifndef AMBULANT_NO_IOSTREAMS
		std::ifstream ifs(m_source->get_url().c_str(), std::ios::in | std::ios::binary);
		if(!ifs) {
			logger::get_logger()->error(repr() + "::read() failed");
			evp->add_event(readdone, 0, ambulant::lib::event_processor::high);
			return;
		}
		const size_t buf_size = 1024;
		byte *buf = new byte[buf_size];
		while(!ifs.eof() && ifs.good()){
			ifs.read((char*)buf, buf_size);
			m_buffer.append(buf, ifs.gcount());
		}
		delete[] buf;
		m_gptr = 0;
#endif
		evp->add_event(readdone, 0, ambulant::lib::event_processor::high);
	}

	size_type size() const { return m_buffer.size();}
	
	size_type available() const { return m_buffer.size() - m_gptr;}
	
	void seekg(size_type pos) { m_gptr = pos;}
	
	const byte* data() const { return  m_buffer.data();}
	
	const byte* gdata() { return m_buffer.data() + m_gptr;}
	
	byte get() { 
		if(!available()) throw_range_error();
		byte b = *gdata(); 
		m_gptr++; 
		return b;
	}
	
	size_type read(byte *b, size_type nb) {
		size_type nr = available();
		size_type nt = (nr>=nb)?nb:nr;
		if(nt>0) {
			memcpy(b, gdata(), nt);
			m_gptr += nt;
		}
		return nt;
	}
	
	size_type skip(size_type nb) {
		size_type nr = available();
		size_type nt = (nr>=nb)?nb:nr;
		if(nt>0) m_gptr += nt;
		return nt;
	}
	
	unsigned short get_be_ushort() {
		byte b[2];
		if(read(b, 2) != 2) throw_range_error();
		return (b[1]<<8)| b[0];
	}
	
#if !defined(AMBULANT_NO_IOSTREAMS) && !defined(AMBULANT_NO_STRINGSTREAM)
	std::string repr() {
		std::ostringstream os;
		os << "active_datasource[" << (m_source?m_source->repr():"NULL") + "]";
		return os.str();
	};
#endif

	size_type read(char *b, size_type nb) {
		return read((byte*)b, nb);
	}
	
#ifndef AMBULANT_NO_IOSTREAMS
	friend inline std::ostream& operator<<(std::ostream& os, const active_datasource& n) {
		os << "active_datasource(" << (void *)&n << ", source=" << (void *)n.m_source << ")";
		return os;
	}
#endif

  private:	
	void throw_range_error() {
		throw std::range_error("index out of range");
	}
	
	passive_datasource *m_source;
	databuffer m_buffer;
	size_type m_gptr;
  
};

//////////////////
// Inline implemetation

inline active_datasource *passive_datasource::activate() { 
	return new active_datasource(this);
}

} // end namespace net

} //end namespace ambulant

#endif // end of (ifndef WIN32 / else / endif) block.

///////////////////////////////////////////////

#endif  //AMBULANT_NET_DATASOURCE_H
