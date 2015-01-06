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

#include "ambulant/lib/memfile.h"

using namespace ambulant;
using namespace lib;

typedef unsigned char byte;

memfile::~memfile() {
	if (m_src) {
		m_src->stop();
		m_src->release();
	}
	m_src = NULL;
}

bool
memfile::read() {
	if (!m_src) return false;
	char *data_;
	size_t datasize;
	if (!net::read_data_from_datasource(m_src, &data_, &datasize))
		return false;
	m_buffer.append((lib::byte*)data_, datasize);
	free(data_);
	m_src->release();
	m_src = NULL;
	return true;
}

memfile::size_type
memfile::size() const
{ return m_buffer.size();}

memfile::size_type
memfile::available() const { return m_buffer.size() - m_gptr;}

void
memfile::seekg(size_type pos) { m_gptr = pos;}

const lib::byte*
memfile::data() const { return	m_buffer.data();}
const lib::byte*
memfile::begin() const { return	 m_buffer.data();}
const lib::byte*
memfile::end() const { return  m_buffer.data() + size();}

const lib::byte*
memfile::gdata() { return m_buffer.data() + m_gptr;}

lib::byte
memfile::get() {
	if(!available())
		throw_range_error();
	byte b = *gdata();
	m_gptr++;
	return b;
}

memfile::size_type
memfile::read(lib::byte *b, size_type nb) {
	size_type nr = available();
	size_type nt = (nr>=nb)?nb:nr;
	if(nt>0) {
		memcpy(b, gdata(), nt);
		m_gptr += nt;
	}
	return nt;
}

memfile::size_type
memfile::skip(size_type nb) {
	size_type nr = available();
	size_type nt = (nr>=nb)?nb:nr;
	if(nt>0) m_gptr += nt;
	return nt;
}

unsigned short
memfile::get_be_ushort() {
	byte b[2];
	if(read(b, 2) != 2)
		throw_range_error();
	return (b[1]<<8)| b[0];
}

memfile::size_type
memfile::read(char *b, size_type nb) {
	return read((byte*)b, nb);
}
