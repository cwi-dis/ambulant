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

/* 
 * @$Id$ 
 */

#include "ambulant/net/datasource.h"
#include "ambulant/net/databuffer.h"
#include "ambulant/net/stdio_datasource.h"
//#include <unistd.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif


// ***********************************  C++  CODE  ***********************************

// data_buffer

using namespace ambulant;

	

net::datasource* 
net::stdio_datasource_factory::new_datasource(const std::string& url)
{
	//XXXX Here we should check if url points to a file or to a network location (rtp/rtsp)
	if (url != "") {
		FILE *fp = fopen(url.c_str(), "rb");
		if (fp == NULL) return NULL;
		return new stdio_datasource(url, fp);
	} else {
		return NULL;
	}
	
}

// *********************** stdio_datasource ***********************************************


net::stdio_datasource::stdio_datasource(const std::string& url, FILE* file)
:	m_buffer(NULL),
	m_url(url),
	m_filesize(0),
	m_stream(file),
	m_end_of_file(false)
{
	if (file >= 0) {
		filesize();
//		m_end_of_file = m_filesize > 0;
		m_buffer = new databuffer(m_filesize);
		if (!m_buffer) {
			m_buffer = NULL;
 			lib::logger::get_logger()->fatal("stdio_datasource(): out of memory");
		}
		//AM_DBG m_buffer->dump(std::cout, false);
	}
}


void
net::stdio_datasource::callback()
{
}

bool
net::stdio_datasource::end_of_file()
{
	if (m_buffer->buffer_not_empty()) return false;
	return m_end_of_file;
}

net::stdio_datasource::~stdio_datasource()
{
	if (m_buffer) {
		delete m_buffer;
		m_buffer = NULL;
	}
	fclose(m_stream);
}

int
net::stdio_datasource::size() const
{
	return m_buffer->size();
}

void
net::stdio_datasource::filesize()
{
 	using namespace std;
	if (m_stream >= 0) {
		// Seek to the end of the file, and get the filesize
		fseek(m_stream, 0, SEEK_END);
		m_filesize = ftell(m_stream);
	 	fseek(m_stream, 0, SEEK_SET);						
	} else {
 		lib::logger::get_logger()->fatal("stdio_datasource.filesize(): no file openXX");
		m_filesize = 0;
	}
}


void
net::stdio_datasource::read(char *data, int size)
{
    char* in_ptr;
    if (size <= m_buffer->size()) {
    	in_ptr = m_buffer->get_read_ptr();
        memcpy(data,in_ptr,size);
        m_buffer->readdone(size);
    }
}

void
net::stdio_datasource::read_file()
{
  	char *buf;
  	size_t n; 	
	//AM_DBG lib::logger::get_logger()->trace("stdio_datasource.readfile: start reading file ");
	if (m_stream >= 0) {
		do {
		//AM_DBG lib::logger::get_logger()->trace("stdio_datasource.readfile: getting buffer pointer");
            buf = m_buffer->get_write_ptr(BUFSIZ);
			//AM_DBG lib::logger::get_logger()->trace("stdio_datasource.readfile: buffer ptr : %x", buf);
			if (buf) {
				//AM_DBG lib::logger::get_logger()->trace("stdio_datasource.readfile: start reading %d bytes", BUFSIZ);
				n = fread(buf, 1, BUFSIZ, m_stream);
				//AM_DBG lib::logger::get_logger()->trace("stdio_datasource.readfile: done reading %d bytes", n);
				if (n > 0) m_buffer->pushdata((int)n); 
			}
		
		} while (n > 0);
		m_end_of_file = true;
		if (n < 0) {
			lib::logger::get_logger()->error("stdio_datasource.read_file(): %s", strerror(errno));
		} 		
	}
}
 
char* 
net::stdio_datasource::get_read_ptr()
{
	return m_buffer->get_read_ptr();
}
  
void
net::stdio_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *callback)
 {
 	if (! end_of_file() ) read_file();
	
	if (m_buffer->size() > 0 ) {
    	if (evp && callback) {
			AM_DBG lib::logger::get_logger()->trace("stdio_datasource.start: trigger readdone callback (x%x)", callback);
			evp->add_event(callback, 0, ambulant::lib::event_processor::high);
    	}
	}
}
 
void
net::stdio_datasource::readdone(int size)
{
	m_buffer->readdone(size);
}
