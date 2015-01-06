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
#include "ambulant/net/win32_datasource.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/win32/win32_error.h"

#include <windows.h>
#include <wininet.h>
#pragma comment (lib,"wininet.lib")

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace net;


class win32_file_datasource : virtual public win32_datasource
{
  public:
	win32_file_datasource(const url& url, HANDLE hf)
	:	win32_datasource(url),
		m_hf(hf)
	{
		m_resourcetype = "file";
		if (hf==NULL) m_end_of_file = true; 
	}
	~win32_file_datasource();
  protected:
	void _read_file();

	HANDLE m_hf;
	long m_bytes_read;
};

class win32_net_datasource : virtual public win32_datasource
{
  public:
	win32_net_datasource(const url& url, HINTERNET hinet, HINTERNET hf)
	:	win32_datasource(url),
		m_hinet(hinet),
		m_hf(hf)
	{ 
		m_resourcetype = "http";
		if (hf==NULL) m_end_of_file = true;
	}
	~win32_net_datasource();
  protected:
	void _read_file();

	HINTERNET m_hinet;
	HINTERNET m_hf;
	long m_bytes_read;
};


#define BUF_SIZE 1024

static HANDLE
open_local_file(const net::url& url)
{
	std::string filename = url.get_file();
	lib::textptr tp(filename.c_str());
	HANDLE hf = CreateFile(tp,
		GENERIC_READ,
		FILE_SHARE_READ,  // 0 = not shared or FILE_SHARE_READ
		0,	// lpSecurityAttributes
		OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY,
		NULL);
	if(hf == INVALID_HANDLE_VALUE) {
		lib::logger::get_logger()->error("%s: Cannot open file", filename.c_str());
		return NULL;
	}
	return hf;
}

static bool
open_net_file(const net::url& url, HINTERNET *hinet_p, HINTERNET *hf_p)
{
	HINTERNET hinet = InternetOpen(text_str("AmbulantPlayer"),
		INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if(!hinet) {
		lib::win32::win_report_last_error("InternetOpen()");
		return false;
	}
	std::string url_str = url.get_url();
	lib::textptr tp(url_str.c_str());
	HINTERNET hf = InternetOpenUrl(hinet, tp,  NULL, 0, INTERNET_FLAG_RAW_DATA, 0);
	if(!hf) {
		lib::logger::get_logger()->trace("%s: InternetOpenUrl returned error 0x%x", url.get_url().c_str(), GetLastError());
		lib::logger::get_logger()->error("%s: Cannot open URL", url.get_url().c_str());
		InternetCloseHandle(hinet);
		return false;
	}
	*hinet_p = hinet;
	*hf_p = hf;
	return true;
}

raw_datasource_factory *
ambulant::net::get_win32_datasource_factory()
{
	return new win32_datasource_factory();
}

datasource *
win32_datasource_factory::new_raw_datasource(const url& url)
{
	if (url.is_local_file()) {
		HANDLE hf = open_local_file(url);
		if (hf)
			return new win32_file_datasource(url, hf);
	// Check for a data: url
	} else if ( ! lib::starts_with(url.get_url(), "data:")) {
		HINTERNET hinet, hf;
		if (open_net_file(url, &hinet, &hf))
			return new win32_net_datasource(url, hinet, hf);
	}
	return NULL;
}

// *********************** win32_datasource ***********************************************


win32_datasource::win32_datasource(const url& url)
:	m_url(url),
	m_buffer(NULL),
	m_bytes_read(0),
	m_resourcetype(NULL),
	m_end_of_file(false)
{
		m_buffer = new databuffer();
		if (m_buffer) {
			m_buffer->set_max_size(0);
		} else {
			m_buffer = NULL;
			lib::logger::get_logger()->fatal("win32_datasource(): out of memory");
		}
}

bool
win32_datasource::end_of_file()
{
	m_lock.enter();
	bool rv = _end_of_file();
	m_lock.leave();
	return rv;
}

bool
win32_datasource::_end_of_file()
{
	// private method - no need to lock
	if (m_buffer->buffer_not_empty()) return false;
	return m_end_of_file;
}

win32_datasource::~win32_datasource()
{
	stop();
	m_lock.enter();
	if (m_buffer) {
		delete m_buffer;
		m_buffer = NULL;
	}
	m_lock.leave();
}

void
win32_datasource::stop()
{
}

size_t
win32_datasource::size() const
{
	// const method - cannot lock
	return m_buffer->size();
}

void
win32_datasource::read(char *data, size_t sz)
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

char*
win32_datasource::get_read_ptr()
{
	m_lock.enter();
	assert(!_end_of_file());
	char * rv = m_buffer->get_read_ptr();
	m_lock.leave();
	return rv;
}

void
win32_datasource::start(ambulant::lib::event_processor *evp, ambulant::lib::event *cbevent)
{
	m_lock.enter();
	if (! _end_of_file() ) _read_file();

	assert(evp);
	assert(cbevent);
	AM_DBG lib::logger::get_logger()->debug("win32_datasource.start: trigger readdone callback (x%x)", cbevent);
	evp->add_event(cbevent, 0, ambulant::lib::ep_med);
	m_lock.leave();
}

void
win32_datasource::readdone(size_t sz)
{
	m_lock.enter();
	m_buffer->readdone(sz);
	m_lock.leave();
}

long
win32_datasource::get_bandwidth_usage_data(const char **resource)
{
	m_lock.enter();
	long rv = m_bytes_read;
	m_bytes_read = 0;
	*resource = m_resourcetype;
	m_lock.leave();
	return rv;
}

void
win32_file_datasource::_read_file()
{
	// private method - no need to lock
	if (m_hf == NULL) return; // Read the data before, or there's an error
	DWORD nread = 0;
	do {
		byte *buf = (byte *)m_buffer->get_write_ptr(BUF_SIZE);
		if (buf == NULL) {
			lib::logger::get_logger()->error("Out of memory");
			break;
		}
		BOOL bres = ReadFile( m_hf, buf, BUF_SIZE, &nread, 0);
		if(!bres) {
			lib::win32::win_report_last_error("InternetReadFile()");
			break;
		}
		m_bytes_read += nread;
		if (nread) m_buffer->pushdata(nread);
	} while(nread > 0);
	m_buffer->pushdata(0);
	// Also close the handles now, why wait until later...
	CloseHandle(m_hf);
	m_hf = NULL;
	m_end_of_file = true;
}

win32_file_datasource::~win32_file_datasource()
{
	if (m_hf) CloseHandle(m_hf);
	m_hf = NULL;
}

void
win32_net_datasource::_read_file()
{
	// private method - no need to lock
	if (m_hf == NULL) return; // Read the data before, or there's an error
	DWORD nread = 0;
	do {
		byte *buf = (byte *)m_buffer->get_write_ptr(BUF_SIZE);
		if (buf == NULL) {
			lib::logger::get_logger()->error("Out of memory");
			break;
		}
		BOOL bres = InternetReadFile(m_hf, buf, BUF_SIZE, &nread);
		if(!bres) {
			lib::win32::win_report_last_error("InternetReadFile()");
			break;
		}
		m_bytes_read += nread;
		if (nread) m_buffer->pushdata(nread);
	} while(nread > 0);
	m_buffer->pushdata(0);
	// Also close the handles now, why wait until later...
	InternetCloseHandle(m_hf);
	m_hf = NULL;
	InternetCloseHandle(m_hinet);
	m_hinet = NULL;
	m_end_of_file = true;
}

win32_net_datasource::~win32_net_datasource()
{
	// close handles
	if (m_hf) InternetCloseHandle(m_hf);
	m_hf = NULL;
	if (m_hinet) InternetCloseHandle(m_hinet);
	m_hinet = NULL;
}
