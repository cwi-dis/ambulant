/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
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
#include "ambulant/net/datasource.h"

#include <string>
#include <stdexcept>

namespace ambulant {

namespace lib {

typedef unsigned char byte;
typedef  std::basic_string<byte> databuffer;

class memfile {
  public:
	typedef std::basic_string<byte> buffer_type;
	typedef buffer_type::value_type value_type;
	typedef buffer_type::pointer pointer;
	typedef buffer_type::const_pointer const_pointer;
	typedef buffer_type::size_type size_type;

	memfile(net::datasource *src)
	:	m_gptr(0),
		m_src(src) {}

	~memfile();

	bool read();

	size_type size() const;

	size_type available() const;

	void seekg(size_type pos);

	const byte* data() const;
	const byte* begin() const;
	const byte* end() const;

	const byte* gdata();

	byte get();

	size_type read(byte *b, size_type nb);

	size_type skip(size_type nb);

	unsigned short get_be_ushort();

	size_type read(char *b, size_type nb);

  private:
	void throw_range_error() {
		throw std::range_error("index out of range");
	}
	std::string m_url;
	databuffer m_buffer;
	size_type m_gptr;
	net::datasource *m_src;

};

} // end namespace lib

} //end namespace ambulant

#endif  // AMBULANT_LIB_MEMFILE_H
