
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

#include "ambulant/net/databuffer.h"
#include "ambulant/lib/logger.h"

#ifndef AMBULANT_PLATFORM_WIN32
#include <unistd.h>
#endif

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

// Define this to put random garbage into newly allocated buffer space
// and into buffer space that has been freed.
#undef RANDOM_BYTES
// ***********************************  C++  CODE  ***********************************

// data_buffer


//#define DEFAULT_MAX_BUF_SIZE 4096
long int ambulant::net::databuffer::s_default_max_size = 1000000;
long int ambulant::net::databuffer::s_default_max_unused_size = 1000000;

using namespace ambulant;
using namespace net;

// Static methods:
void
databuffer::default_max_size(int max_size)
{
	s_default_max_size = max_size;
}

void
databuffer::default_max_unused_size(int max_unused_size)
{
	s_default_max_unused_size = max_unused_size;
}

databuffer::databuffer()
{
	AM_DBG lib::logger::get_logger()->debug("databuffer::databuffer() -> 0x%x", (void*)this);
	m_size = 0;
    m_used = 0;
    m_rear = 0;
    m_max_size = s_default_max_size;
	m_max_unused_size = s_default_max_unused_size;
	//AM_DBG lib::logger::get_logger()->debug("active_datasource.databuffer(): [size = %d, max size = %d]",m_size, m_max_size);
	m_buffer = NULL;
    m_buffer_full = false;
}

bool
databuffer::buffer_full()
{
	return m_buffer_full;
}


bool
databuffer::buffer_not_empty()
{
    if (m_used > 0) {
        return true;
    } else {
        return false;
    }
}
            


databuffer::databuffer(int max_size)
{
	AM_DBG lib::logger::get_logger()->debug("databuffer::databuffer(max_size=%d) -> 0x%x", max_size, (void*)this);
    m_used = 0;
    m_size = 0;
    m_rear = 0;
	m_buffer = NULL;
    set_max_size(max_size);
}

void
databuffer::set_max_size(int max_size)
{
	m_lock.enter();
	// Zero means: no limit, <0 means: default
    if (max_size >= 0) {
        m_max_size = max_size;
    } else {
        m_max_size = s_default_max_size;
    }
    m_buffer_full = (m_max_size > 0 && m_used > m_max_size);
	if (m_buffer_full) lib::logger::get_logger()->debug("databuffer::set_max_size(0x%x, %d): buffer now full (used=%d)",
		(void*)this, max_size, m_used);
	m_lock.leave();
}

databuffer::~databuffer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("databuffer::~databuffer(0x%x)", (void*)this);
	if (m_buffer) {
		free(m_buffer);
		m_used = 0;
		m_size = 0;
        m_rear = 0;
        m_buffer = NULL;
	}
	m_lock.leave();
}

int databuffer::size() const
{
	return m_used;
}

void databuffer::dump(std::ostream& os, bool verbose) const
{
	unsigned long int i;

	os << "BUFFER SIZE : " << m_size << " bytes" << std::endl;
	os << "BYTES USED : " << m_used << " bytes" << std::endl;
	os << "m_rear   : " << m_rear << std::endl;
	if (verbose) {
		if (m_buffer) {
			for (i = m_rear;i < m_size;i++) {
	   		os << m_buffer[i];
	   		}
		}
	} 
 	os << std::endl;
}

char *
databuffer::get_write_ptr(int sz)
{
	m_lock.enter();
	char *rv = NULL;
	AM_DBG lib::logger::get_logger()->debug("databuffer(0x%x).get_write_ptr(%d): start ", (void*)this, sz);
	
    if(!m_buffer_full) {
        m_buffer = (char*) realloc(m_buffer, m_size + sz);
		AM_DBG lib::logger::get_logger()->debug("databuffer::get_write_ptr: buffer realloc done (%x)",m_buffer);
        if (!m_buffer) {
            lib::logger::get_logger()->fatal("databuffer::databuffer(size=%d): out of memory", m_size+sz);
        }
#ifdef RANDOM_BYTES
		unsigned int i;
		for(i=m_size; i<m_size+sz; i++) m_buffer[i] = (char) rand();
#endif
		//AM_DBG lib::logger::get_logger()->debug("databuffer.get_write_ptr: returning m_front (%x)",m_buffer + m_size);
		rv = m_buffer + m_size;
    } else {
        lib::logger::get_logger()->warn("databuffer::databuffer::get_write_ptr : buffer full but still trying to obtain write pointer ");
		rv = NULL;
    }
    m_lock.leave();
	return rv;
}

void databuffer::pushdata(int sz)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("databuffer(0x%x)::pushdata(%d) m_size=%d", (void*)this, sz, m_size);
	if (m_buffer_full) {
        lib::logger::get_logger()->warn("databuffer::databuffer::pushdata : buffer full but still trying to fill it");
    }
	m_size += sz;
	//AM_DBG lib::logger::get_logger()->debug("active_datasource.pushdata:size = %d ",sz);
	m_used = m_size - m_rear;
	if (sz < 0 || m_size <= 0) { // cannot realloc XXXX Is this OK ?
	  m_lock.leave();
	  return;
	}
	m_buffer = (char*) realloc(m_buffer, m_size);
	 if (!m_buffer) {
		 lib::logger::get_logger()->fatal("databuffer::databuffer(size=%d): out of memory", m_size);
	 }
	if(m_max_size > 0 && m_used > m_max_size) {
		AM_DBG lib::logger::get_logger()->debug("active_datasource.pushdata: buffer full [size = %d, max size = %d]",m_size, m_max_size);
		m_buffer_full = true;
	}
	m_lock.leave();
}


char *
databuffer::get_read_ptr()
{
	m_lock.enter();
	char *rv = (m_buffer + m_rear);
	AM_DBG lib::logger::get_logger()->debug("databuffer(0x%x)::get_read_ptr(): returning 0x%x", (void*)this, (void*)rv);
	m_lock.leave();
	return rv;
}

void
databuffer::readdone(int sz)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("databuffer(0x%x)::readdone(%d)", (void*)this, sz);
    if ((unsigned long int)sz > m_used) {
		lib::logger::get_logger()->error("databuffer::readdone(%d), but m_used=%d", sz, m_used);
		sz = m_used;
	}
#ifdef RANDOM_BYTES
	unsigned int i;
	for(i=m_rear; i<m_rear+sz; i++) m_buffer[i] = (char) rand();
#endif
	m_rear += sz;
	m_used = m_size - m_rear;
	m_buffer_full = (m_max_size > 0 && m_used > m_max_size);
	if (m_used == 0 || (m_max_unused_size > 0 && m_rear > m_max_unused_size)) {
		 // Free the unused space in the buffer
		 if (m_used) memcpy(m_buffer, m_buffer+m_rear, m_used);
		 m_buffer = (char *)realloc(m_buffer, m_used);
		 m_size = m_used;
		 m_rear = 0;
		 if (m_buffer == NULL && m_used > 0) {
			 lib::logger::get_logger()->fatal("databuffer::databuffer(size=%d): out of memory", m_size);
		 }
	}
	m_lock.leave();
}
