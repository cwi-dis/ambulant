/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

 
#ifndef AMBULANT_LIB_MEMFILE_H
#define AMBULANT_LIB_MEMFILE_H

#include "ambulant/config/config.h"

#include <string>

#if !defined(AMBULANT_NO_IOSTREAMS) && !defined(AMBULANT_PLATFORM_WIN32)
#include <sstream>
#include <fstream>
#elif defined(AMBULANT_PLATFORM_WIN32) 
#include "ambulant/lib/win32/win32_memfile.h"
#endif

// debug
#include "ambulant/lib/logger.h"

namespace ambulant {

namespace lib {

typedef unsigned char byte;
typedef  std::basic_string<byte> databuffer;
using ambulant::lib::logger;

#if !defined(AMBULANT_NO_IOSTREAMS_HEADERS) && !defined(AMBULANT_PLATFORM_WIN32)

class memfile {
  public:
	typedef std::basic_string<byte> buffer_type;
	typedef buffer_type::value_type value_type;
	typedef buffer_type::pointer pointer;
	typedef buffer_type::const_pointer const_pointer;
	typedef buffer_type::size_type size_type;

	memfile(const std::string& url)
	:	m_url(url), m_gptr(0) {
	}
	
	memfile(const char *url)
	:	m_url(url?url:""), m_gptr(0) {
	}
	
	~memfile() {
	}
	
	bool exists() const {
		return memfile::exists(m_url);
	}
	
	databuffer& get_databuffer() { return m_buffer;}
	
	const std::string& get_url() const { return m_url;}
	
	bool read() {
		std::ifstream ifs(m_url.c_str(), std::ios::in | std::ios::binary);
		if(!ifs) {
			logger::get_logger()->error(repr() + "::read() failed");
			return false;
		}
		const size_t buf_size = 1024;
		byte *buf = new byte[buf_size];
		while(!ifs.eof() && ifs.good()){
			ifs.read((char*)buf, buf_size);
			m_buffer.append(buf, ifs.gcount());
		}
		delete[] buf;
		m_gptr = 0;
		return true;
	}

	size_type size() const { return m_buffer.size();}
	
	size_type available() const { return m_buffer.size() - m_gptr;}
	
	void seekg(size_type pos) { m_gptr = pos;}
	
	const byte* data() const { return  m_buffer.data();}
	const byte* begin() const { return  m_buffer.data();}
	const byte* end() const { return  m_buffer.data() + size();}
	
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
	
	std::string repr() {
		std::string s("memfile[");
		s += (!m_url.empty()?m_url:"NULL");
		s += "]";
		return s;
	};

	size_type read(char *b, size_type nb) {
		return read((byte*)b, nb);
	}
	
	static bool exists(const std::string& url) {
		std::ifstream ifs(url.c_str());
		bool exists = ifs && ifs.good();
		ifs.close();
		return exists;
	}
	
  private:	
	void throw_range_error() {
		throw std::range_error("index out of range");
	}
	std::string m_url;
	databuffer m_buffer;
	size_type m_gptr;
  
};

#elif defined(AMBULANT_PLATFORM_WIN32)
using ambulant::lib::win32::memfile;
#endif

} // end namespace lib

} //end namespace ambulant

#endif  // AMBULANT_LIB_MEMFILE_H
