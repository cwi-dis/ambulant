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
#include "ambulant/net/posix_datasource.h"
#include <unistd.h>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif


// ***********************************  C++  CODE  ***********************************

// data_buffer

using namespace ambulant;
using namespace net;

datasource* 
posix_datasource_factory::new_raw_datasource(const net::url& url)
{
	AM_DBG lib::logger::get_logger()->debug("posix_datasource_factory::new_datasource(%s)", repr(url).c_str());
	if (url.is_local_file()) {

		passive_datasource *pds = new passive_datasource(url.get_file());
		return pds->activate();
	} else {
		return NULL;
	}
	
}


// *********************** passive_datasource ***********************************************



datasource* 
passive_datasource::activate()
{
	int in;
	
	in = open(m_filename.c_str(), O_RDONLY);
	if (in >= 0) {
		return new active_datasource(this, in);
	} else {
		lib::logger::get_logger()->trace(gettext("%s: Cannot open: %s"),  m_filename.c_str(), strerror(errno));
	}
	return NULL;
		
}

passive_datasource::~passive_datasource()
{
	AM_DBG lib::logger::get_logger()->debug("passive_datasource::~passive_datasource(0x%x)", (void*)this);
}

// *********************** active_datasource ***********************************************


active_datasource::active_datasource(passive_datasource *const source, int file)
:	m_buffer(NULL),
	m_source(source),
	m_filesize(0),
	m_stream(file),
	m_end_of_file(false)
{
	AM_DBG lib::logger::get_logger()->debug("active_datasource::active_datasource(0x%x, %d)->0x%x", (void*)source, file, (void*)this);
	m_source->add_ref();
	if (file >= 0) {
		filesize();
//		m_end_of_file = m_filesize > 0;
		m_buffer = new databuffer(m_filesize);
		if (!m_buffer) {
			m_buffer = NULL;
 			lib::logger::get_logger()->fatal("active_datasource(): out of memory");
		}
		//AM_DBG m_buffer->dump(std::cout, false);
	}
}


void
active_datasource::callback()
{
}

bool
active_datasource::end_of_file()
{
	bool rv;
	m_lock.enter();
	rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool
active_datasource::_end_of_file()
{
	bool rv;
	// private method - no need to lock
	if (m_buffer->buffer_not_empty()) 
		rv = false;
	else
		rv = m_end_of_file;
	return rv;
}

active_datasource::~active_datasource()
{
	stop();
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("active_datasource::stop(0x%x)", (void*)this);
	if (m_buffer) {
		delete m_buffer;
		m_buffer = NULL;
	}
	if (m_source) m_source->release();
	m_source = NULL;
	close(m_stream);
	m_lock.leave();
}

void
active_datasource::stop()
{
}

int
active_datasource::size() const
{
	// const method - cannot use the lock.
	int rv = m_buffer->size();
	return rv;
}

void
active_datasource::filesize()
{
	// private method - no need to lock
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
active_datasource::read(char *data, int sz)
{
	m_lock.enter();
    char* in_ptr;
    if (sz <= m_buffer->size()) {
    	in_ptr = m_buffer->get_read_ptr();
        memcpy(data,in_ptr,sz);
        m_buffer->readdone(sz);
    }
	m_lock.leave();
}

void
active_datasource::read_file()
{
	// private method - no need to lock
  	char *buf;
  	int n; 	
	//AM_DBG lib::logger::get_logger()->debug("active_datasource.readfile: start reading file ");
	if (m_stream >= 0) {
		do {
		//AM_DBG lib::logger::get_logger()->debug("active_datasource.readfile: getting buffer pointer");
            buf = m_buffer->get_write_ptr(BUFSIZ);
			//AM_DBG lib::logger::get_logger()->debug("active_datasource.readfile: buffer ptr : %x", buf);
			if (buf) {
				//AM_DBG lib::logger::get_logger()->debug("active_datasource.readfile: start reading %d bytes", BUFSIZ);
				n = ::read(m_stream, buf, BUFSIZ);
				//AM_DBG lib::logger::get_logger()->debug("active_datasource.readfile: done reading %d bytes", n);
				if (n > 0) m_buffer->pushdata(n); 
			}
		
		} while (n > 0);
		m_end_of_file = true;
		if (n < 0) {
			lib::logger::get_logger()->trace("%s: %s", m_source->m_filename.c_str(), strerror(errno));
			lib::logger::get_logger()->warn(gettext("Error encountered while reading file %s"), m_source->m_filename.c_str());
		} 		
	}
}
 
char* 
active_datasource::get_read_ptr()
{
	m_lock.enter();
	char *rv = m_buffer->get_read_ptr();
	m_lock.leave();
	return rv;
}
  
void
active_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *cbevent)
 {
	m_lock.enter();
 	if (! _end_of_file() ) read_file();
	
	if (m_buffer->size() > 0 ) {
    	if (evp && cbevent) {
			AM_DBG lib::logger::get_logger()->debug("active_datasource.start: trigger readdone callback (x%x)", cbevent);
			evp->add_event(cbevent, 0, ambulant::lib::event_processor::med);
    	}
	}
	m_lock.leave();
}
 
void
active_datasource::readdone(int sz)
{
	m_lock.enter();
	m_buffer->readdone(sz);
	m_lock.leave();
}
