
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


// ***********************************  C++  CODE  ***********************************

// data_buffer


//#define DEFAULT_MAX_BUF_SIZE 4096
#define DEFAULT_MAX_BUF_SIZE 1000000

using namespace ambulant;
using namespace net;

databuffer::databuffer()
{
	m_size = 0;
    m_used = 0;
    m_rear = 0;
    m_max_size = DEFAULT_MAX_BUF_SIZE;
	//AM_DBG lib::logger::get_logger()->trace("active_datasource.databuffer(): [size = %d, max size = %d]",m_size, m_max_size);
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
    m_used = 0;
    m_size = 0;
    m_rear = 0;
	m_buffer = NULL;
    if (max_size > 0) {
        m_max_size = max_size;
    } else {
        m_max_size = DEFAULT_MAX_BUF_SIZE;
    }
	//AM_DBG lib::logger::get_logger()->trace("active_datasource.databuffer(%d): [size = %d, max size = %d]",max_size, m_size, m_max_size);
    m_buffer_full = false;
}


databuffer::~databuffer()
{
	if (m_buffer) {
		free(m_buffer);
		m_used = 0;
		m_size = 0;
        m_rear = 0;
        m_buffer = NULL;
	}
}

int databuffer::size() const
{
	return m_used;
}

void databuffer::dump(std::ostream& os, bool verbose) const
{
	int i;

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
databuffer::get_write_ptr(int size)
{
	//AM_DBG lib::logger::get_logger()->trace("databuffer.get_write_ptr: start BUFSIZ = %d", BUFSIZ);
	
    if(!m_buffer_full) {
        m_buffer = (char*) realloc(m_buffer, m_size + size);
		//AM_DBG lib::logger::get_logger()->trace("databuffer.prepare: buffer realloc done (%x)",m_buffer);
        if (!m_buffer) {
            lib::logger::get_logger()->fatal("databuffer::databuffer(size=%d): out of memory", m_size);
        }
		//AM_DBG lib::logger::get_logger()->trace("databuffer.get_write_ptr: returning m_front (%x)",m_buffer + m_size);
		return (m_buffer + m_size);
    } else {
        lib::logger::get_logger()->warn("databuffer::databuffer::get_write_ptr : buffer full but still trying to fill it ");
		return NULL;
    }
    
}

void databuffer::pushdata(int size)
{
    if(!m_buffer_full) {
        m_size += size;
		//AM_DBG lib::logger::get_logger()->trace("active_datasource.pushdata:size = %d ",size);
        m_used = m_size - m_rear;
        m_buffer = (char*) realloc(m_buffer, m_size);
         if (!m_buffer) {
             lib::logger::get_logger()->fatal("databuffer::databuffer(size=%d): out of memory", m_size);
         }
        if(m_size > m_max_size) {
			AM_DBG lib::logger::get_logger()->trace("active_datasource.pushdata: buffer full [size = %d, max size = %d]",m_size, m_max_size);
            m_buffer_full = true;
        }
    } else {
        lib::logger::get_logger()->warn("databuffer::databuffer::pushdata : buffer full but still trying to fill it");
    }
}


char *
databuffer::get_read_ptr()
{
	return (m_buffer + m_rear);
}

void
databuffer::readdone(int size)
{
    if (size <= m_used) {
        m_rear += size;
        m_used = m_size - m_rear;
    }
}
