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
#include <unistd.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif


// ***********************************  C++  CODE  ***********************************

// data_buffer

using namespace ambulant;

	
net::databuffer::databuffer()
{
	m_size = 0;
	m_used = 0;
	m_buffer = NULL;
}


void net::databuffer::resize(int newsize)
{
	int m_dummy;
	char *m_newbuf;
	m_newbuf = new char[newsize];
	if (m_newbuf) {
		// first copy the buffer
		if (m_size > newsize) {
			m_dummy = newsize;
		} else { 
			m_dummy = m_size;
		}
	
		memcpy(m_newbuf, m_buffer, newsize);
	
		// delete the old buffer		
		if (m_buffer) {
				delete[] m_buffer;
			}
		m_size = newsize;
		if (m_used > newsize) m_used = newsize;
		m_buffer = m_newbuf;
	}
}

net::databuffer::databuffer(int size)
{
m_size = 0;
m_used = 0;
m_buffer = new char[size];
if (!m_buffer) {
	lib::logger::get_logger()->fatal("databuffer::databuffer(size=%d): out of memory", size);
	} else {
	m_size = size;
	m_used = 0;
	memset(m_buffer, 0, size);
	}
}

// copy constructor

net::databuffer::databuffer(databuffer& src)   // copy constructor
{
int i;
m_size = 0;
m_used = 0;
m_buffer = new char[src.m_size]; 
if (!m_buffer) {
	lib::logger::get_logger()->fatal("databuffer::databuffer(size=%d): out of memory", src.m_size);
	} else {
	for (i=0;i<src.m_size;i++) {
		m_buffer[i] = src.m_buffer[i];
		}
	m_size = src.m_size;
	m_used = src.m_used;
	}
}


net::databuffer::~databuffer()
{
	if (m_buffer) {
		delete [] m_buffer;
		m_used = 0;
		m_size = 0;
		m_buffer = NULL;
		}
}

int net::databuffer::used() const
{
	return m_used;
}

void net::databuffer::dump(std::ostream& os, bool verbose) const
{
int i;

os << "BUFFER SIZE : " << m_size << " bytes" << std::endl;
os << "BYTES USED : " << m_used << " bytes" << std::endl;
if (verbose) {
	if (m_buffer) {
		for (i = 0;i < m_used;i++) {
	   		os << m_buffer[i];
	   		}
	   	}
	} 
 os << std::endl;
}

void net::databuffer::put_data(char *data, int size)
{
int dummy;

dummy = m_used + size;

if (dummy <= m_size) {
	memcpy((m_buffer+m_used), data, size);
        m_used = m_used +size;
	} else {
	lib::logger::get_logger()->error("databuffer::put_data(size=%d): no room", size);
	}

}

void net::databuffer::shift_down(int pos)
{

if (pos <  m_used) {
	memmove(m_buffer, (m_buffer+pos),  m_used-pos);
	m_used =m_used-pos;
    memset((m_buffer+m_used), 0, m_size-m_used);
    } else if (pos == m_used) {
    m_used=0;
    memset(m_buffer, 0, m_size);
    } else {
	lib::logger::get_logger()->error("databuffer::shift_down(int pos=%d): pos larger then m_used(%d)", pos);
    }
}


void
net::databuffer::get_data(char *data, int size)
{
if (size <= m_used) {		
	memcpy(data, m_buffer, size);
	shift_down(size);
	} else {
		lib::logger::get_logger()->error("databuffer::get_data(size=%d): only %d available", size, m_used);
	}
}



// *********************** passive_datasource ***********************************************



net::active_datasource* net::passive_datasource::activate()
{
	int in;
	
	in = open(m_url.c_str(), O_RDONLY);
	if (in >= 0) {
		return new active_datasource(this, in);
	} else {
		lib::logger::get_logger()->error("passive_datasource.activate(url=\"%s\"): %s",  m_url.c_str(), strerror(errno));
	}
	return NULL;
		
}

net::passive_datasource::~passive_datasource()
{
}

// *********************** active_datasource ***********************************************


net::active_datasource::active_datasource(passive_datasource *const source, int file)
:	m_source(source)
{
	if (file >= 0) {
		m_stream = file;
		filesize();
		m_source = source;
		m_buffer = new databuffer(m_filesize);
		if (!m_buffer) {
			m_buffer = NULL;
 			lib::logger::get_logger()->fatal("active_datasource(): out of memory");
		}
		m_source->add_ref();
		AM_DBG m_buffer->dump(std::cout, false);
	}
}

net::active_datasource::~active_datasource()
{
	if (m_buffer) {
		delete m_buffer;
		m_buffer = NULL;
	}
	m_source->release();
	close(m_stream);
}

int
net::active_datasource::size() const
{
	return m_buffer->used();
}

void
net::active_datasource::filesize()
{
 		using namespace std;
		int dummy;
		if (m_stream >= 0) {
			// Seek to the end of the file, and get the filesize
			m_filesize=lseek(m_stream, 0, SEEK_END); 		
	 		dummy=lseek(m_stream, 0, SEEK_SET);						
			} else {
 			lib::logger::get_logger()->fatal("active_datasource.filesize(): no file openXX");
			m_filesize = 0;
			}
}

void
net::active_datasource::read_file()
{
  	char buf[BUFSIZ];
  	int result; 	
	if (m_stream >= 0) {

		do {
			result = ::read(m_stream, buf, sizeof(buf));
			if (result >0) m_buffer->put_data(buf, result); 
		} while (result > 0);

		if (result < 0) {
			lib::logger::get_logger()->error("active_datasource.read_file(): %s", strerror(errno));
			} 		
	}
}
  
  
void
net::active_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *readdone)
 {
 	read_file();
 	AM_DBG m_buffer->dump(std::cout, false);
	if (evp && readdone) {
		AM_DBG lib::logger::get_logger()->trace("active_datasource.start: trigger readdone callback");
		evp->add_event(readdone, 0, ambulant::lib::event_processor::high);
	}
}
 
void
net::active_datasource::read(char *data, int size)
{
	m_buffer->get_data(data, size);
}

