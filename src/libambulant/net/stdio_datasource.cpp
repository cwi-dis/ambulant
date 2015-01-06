// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/net/datasource.h"
#include "ambulant/net/databuffer.h"
#include "ambulant/net/stdio_datasource.h"
//#include <unistd.h>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace net;

raw_datasource_factory *
ambulant::net::create_stdio_datasource_factory()
{
	return new stdio_datasource_factory();
}

datasource *
stdio_datasource_factory::new_raw_datasource(const url& url)
{
	//XXXX Here we should check if url points to a file or to a network location (rtp/rtsp)
	if (url.is_local_file()) {
		std::string filename = url.get_file();
		FILE *fp = fopen(filename.c_str(), "rb");
		if (fp == NULL) return NULL;
		return new stdio_datasource(url, fp);
	} else {
		return NULL;
	}

}

// *********************** stdio_datasource ***********************************************


stdio_datasource::stdio_datasource(const url& url, FILE* file)
:	m_url(url),
	m_buffer(NULL),
	m_filesize(0),
	m_stream(file),
	m_end_of_file(false)
{
	if (file >= 0) {
		filesize();
//		m_end_of_file = m_filesize > 0;
		assert((size_t)m_filesize == m_filesize);
		m_buffer = new databuffer((size_t)m_filesize);
		if (!m_buffer) {
			m_buffer = NULL;
			lib::logger::get_logger()->fatal("stdio_datasource(): out of memory");
		}
		//AM_DBG m_buffer->dump(std::cout, false);
	}
}

bool
stdio_datasource::end_of_file()
{
	m_lock.enter();
	bool rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool
stdio_datasource::_end_of_file()
{
	// private method - no need to lock
	if (m_buffer->buffer_not_empty()) return false;
	return m_end_of_file;
}

stdio_datasource::~stdio_datasource()
{
	stop();
	m_lock.enter();
	if (m_buffer) {
		delete m_buffer;
		m_buffer = NULL;
	}
	fclose(m_stream);
	m_lock.leave();
}

void
stdio_datasource::stop()
{
}

size_t
stdio_datasource::size() const
{
	// const method - cannot lock
	return m_buffer->size();
}

void
stdio_datasource::filesize()
{
	// private method - no need to lock
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
stdio_datasource::read(char *data, size_t sz)
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
stdio_datasource::read_file()
{
	// private method - no need to lock
	char *buf;
	size_t n;
	//AM_DBG lib::logger::get_logger()->debug("stdio_datasource.readfile:	reading file ");
	if (m_stream >= 0) {
		do {
			buf = m_buffer->get_write_ptr(BUFSIZ);
			assert(buf);
			n = fread(buf, 1, BUFSIZ, m_stream);
			m_buffer->pushdata(n > 0 ? n : 0);
		} while (n > 0);
		m_end_of_file = true;
		if (ferror(m_stream)) {
			lib::logger::get_logger()->trace("%s: %s", m_url.get_url().c_str(), strerror(errno));
			lib::logger::get_logger()->warn(gettext("Error encountered while reading file %s"), m_url.get_url().c_str());
		}
	}
}

char*
stdio_datasource::get_read_ptr()
{
	m_lock.enter();
	assert(!_end_of_file());
	char * rv = m_buffer->get_read_ptr();
	m_lock.leave();
	return rv;
}

void
stdio_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *cbevent)
{
	m_lock.enter();
	if (! _end_of_file() ) read_file();

	assert(evp);
	assert(cbevent);
	AM_DBG lib::logger::get_logger()->debug("stdio_datasource.start: trigger readdone callback (x%x)", cbevent);
	evp->add_event(cbevent, 0, ambulant::lib::ep_med);
	m_lock.leave();
}

void
stdio_datasource::readdone(size_t sz)
{
	m_lock.enter();
	m_buffer->readdone(sz);
	m_lock.leave();
}


long
stdio_datasource::get_bandwidth_usage_data(const char **resource)
{
    m_lock.enter();
    long rv = m_filesize;
    m_filesize = 0;
    *resource = "file";
    m_lock.leave();
    return rv;
}
