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

 
#ifndef AMBULANT_LIB_WIN32_MEMFILE_H
#define AMBULANT_LIB_WIN32_MEMFILE_H

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

#include "ambulant/config/config.h"
#include "ambulant/net/url.h"

#include <string>

namespace ambulant {

namespace lib {

namespace win32 {

typedef unsigned char byte;
typedef  std::basic_string<byte> databuffer;

class memfile {
  public:
	typedef std::basic_string<byte> buffer_type;
	typedef buffer_type::value_type value_type;
	typedef buffer_type::pointer pointer;
	typedef buffer_type::const_pointer const_pointer;
	typedef buffer_type::size_type size_type;

	memfile(const net::url& u);
	~memfile();
	
	bool exists() const { return memfile::exists(m_url);}
	
	databuffer& get_databuffer() { return m_buffer;}
	
	const net::url& get_url() const { return m_url;}
	
	bool read();

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
	
	std::string repr();

	size_type read(char *b, size_type nb) {
		return read((byte*)b, nb);
	}
	static bool exists(const net::url& url);
	
  private:	
	bool read_local(const std::string& fn);
	bool read_remote(const std::string& urlstr);
	bool read_remote();
	void throw_range_error();
	net::url m_url;
	databuffer m_buffer;
	size_type m_gptr;
};


} // end namespace win32

} // end namespace lib

} //end namespace ambulant

#endif  // AMBULANT_LIB_WIN32_MEMFILE_H
