/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
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

/* 
 * @$Id$ 
 */
 
/* 
 * Basic input and output stream interfaces.
 */
 
#ifndef AMBULANT_LIB_AMSTREAM_H
#define AMBULANT_LIB_AMSTREAM_H

#include "ambulant/config/config.h"
#include "ambulant/lib/byte_buffer.h"

#ifndef AMBULANT_NO_IOSTREAMS
#include <iostream>
#endif

namespace ambulant {

namespace lib {

class AMBULANTAPI istream {
  public:
	virtual ~istream() {}
	virtual bool is_open() const = 0;
	virtual void close() = 0;
	virtual void read(byte_buffer& bb) = 0;
	virtual int read(unsigned char *buffer, int nbytes) = 0;
};

class AMBULANTAPI ostream {
  public:
	virtual ~ostream() {}
	virtual bool is_open() const = 0;
	virtual void close() = 0;
	virtual int write(const unsigned char *buffer, int nbytes) = 0;
	virtual int write(const char *cstr) = 0;
	virtual void write(byte_buffer& bb) = 0;
	virtual void flush() = 0;
};

inline ostream& operator<<(ostream& os, const std::string& s) { 
	os.write(reinterpret_cast<const unsigned char*>(s.c_str()), (int)s.length()); return os;}
inline ostream& operator<<(ostream& os, const char *cstr) { os.write(cstr); return os;}

#ifndef AMBULANT_NO_IOSTREAMS
class std_ostream : public ostream {
  public:
	std::ostream& m_os;
	std_ostream(std::ostream& os) : m_os(os) {}
	virtual bool is_open() const {return true;}
	virtual void close() {}
	virtual int write(const unsigned char *buffer, int nbytes) {
		m_os.write((const char*)buffer, nbytes);
		return nbytes;
	}
	virtual int write(const char *cstr) {
		int len = (int)strlen(cstr);
		m_os.write(cstr, len);
		return len;
	}
	virtual void write(byte_buffer& bb) {
		int nwritten = write(bb.data()+bb.get_position(), bb.remaining());
		bb.set_position(bb.get_position()+nwritten);
	}
	virtual void flush() {
		m_os.flush();
	}
};
#endif


} // namespace lib
 
} // namespace ambulant


#endif // AMBULANT_LIB_AMSTREAM_H
