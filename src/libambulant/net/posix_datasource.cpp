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
#include "ambulant/net/posix_datasource.h"
#include <unistd.h>

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif


using namespace ambulant;
using namespace net;

raw_datasource_factory *
ambulant::net::create_posix_datasource_factory()
{
	return new posix_datasource_factory();
}

datasource *
posix_datasource_factory::new_raw_datasource(const net::url& url)
{
	AM_DBG lib::logger::get_logger()->debug("posix_datasource_factory::new_datasource(%s)", repr(url).c_str());
	if (url.is_local_file()) {
		std::string filename = url.get_file();
		int in = open(filename.c_str(), O_RDONLY);
		if (in >= 0)
			return new posix_datasource(filename, in);
	}
	return NULL;
}

// *********************** posix_datasource ***********************************************


posix_datasource::posix_datasource(std::string filename, int file)
:	m_filename(filename),
	m_buffer(NULL),
	m_filesize(0),
	m_stream(file),
	m_end_of_file(false)
{
	AM_DBG lib::logger::get_logger()->debug("posix_datasource::posix_datasource(%s, %d)->0x%x", filename.c_str(), file, (void*)this);
	if (file >= 0) {
		filesize();
//		m_end_of_file = m_filesize > 0;
		m_buffer = new databuffer(m_filesize);
		if (!m_buffer) {
			m_buffer = NULL;
			lib::logger::get_logger()->fatal("posix_datasource(): out of memory");
		}
		//AM_DBG m_buffer->dump(std::cout, false);
	}
}

bool
posix_datasource::end_of_file()
{
	bool rv;
	m_lock.enter();
	rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool
posix_datasource::_end_of_file()
{
	bool rv;
	// private method - no need to lock
	if (m_buffer->buffer_not_empty())
		rv = false;
	else
		rv = m_end_of_file;
	return rv;
}

posix_datasource::~posix_datasource()
{
	stop();
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("posix_datasource::stop(0x%x)", (void*)this);
	if (m_buffer) {
		delete m_buffer;
		m_buffer = NULL;
	}
	close(m_stream);
	m_lock.leave();
}

void
posix_datasource::stop()
{
}

size_t
posix_datasource::size() const
{
	// const method - cannot use the lock.
	int rv = m_buffer->size();
	return rv;
}

void
posix_datasource::filesize()
{
	// private method - no need to lock
	using namespace std;
	off_t dummy;
	if (m_stream >= 0) {
		// Seek to the end of the file, and get the filesize
		m_filesize=lseek(m_stream, 0, SEEK_END);
		dummy=lseek(m_stream, 0, SEEK_SET);
	} else {
		lib::logger::get_logger()->fatal("posix_datasource.filesize(): no file openXX");
		m_filesize = 0;
	}
}


void
posix_datasource::read(char *data, size_t sz)
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
posix_datasource::read_file()
{
	// private method - no need to lock
	char *buf;
	int n;
	//AM_DBG lib::logger::get_logger()->debug("posix_datasource.readfile: start reading file ");
	if (m_stream >= 0) {
		do {
			buf = m_buffer->get_write_ptr(BUFSIZ);
			assert(buf);
			n = ::read(m_stream, buf, BUFSIZ);
			m_buffer->pushdata(n > 0 ? n : 0);
		} while (n > 0);
		m_end_of_file = true;
		if (n < 0) {
			lib::logger::get_logger()->trace("%s: %s", m_filename.c_str(), strerror(errno));
			lib::logger::get_logger()->warn(gettext("Error encountered while reading file %s"), m_filename.c_str());
		}
	}
}

char*
posix_datasource::get_read_ptr()
{
	m_lock.enter();
	assert(!_end_of_file());
	char *rv = m_buffer->get_read_ptr();
	m_lock.leave();
	return rv;
}

void
posix_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *cbevent)
{
	m_lock.enter();
	if (! _end_of_file() ) read_file();

	assert(evp);
	assert(cbevent);
	AM_DBG lib::logger::get_logger()->debug("posix_datasource.start: trigger readdone callback (x%x)", cbevent);
	evp->add_event(cbevent, 0, ambulant::lib::ep_med);
	m_lock.leave();
}

void
posix_datasource::readdone(size_t sz)
{
	m_lock.enter();
	m_buffer->readdone(sz);
	m_lock.leave();
}

long
posix_datasource::get_bandwidth_usage_data(const char **resource)
{
    m_lock.enter();
    long rv = m_filesize;
    m_filesize = 0;
    *resource = "file";
    m_lock.leave();
    return rv;
}

