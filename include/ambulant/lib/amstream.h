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

/*
* Basic input and output stream interfaces.
*/

#ifndef AMBULANT_LIB_AMSTREAM_H
#define AMBULANT_LIB_AMSTREAM_H

#include <string.h> // needed for strlen()
#include "ambulant/config/config.h"
#include <string.h>

#include <iostream>

namespace ambulant {

namespace lib {

class AMBULANTAPI istream {
public:
virtual ~istream() {}
virtual bool is_open() const = 0;
virtual void close() = 0;
virtual int read(unsigned char *buffer, int nbytes) = 0;
};

class AMBULANTAPI ostream {
public:
virtual ~ostream() {}
virtual bool is_open() const = 0;
virtual void close() = 0;
virtual int write(const unsigned char *buffer, int nbytes) = 0;
virtual int write(const char *cstr) = 0;
virtual void flush() = 0;
};

inline ostream& operator<<(ostream& os, const std::string& s) {
os.write(reinterpret_cast<const unsigned char*>(s.c_str()), (int)s.length()); return os;}
inline ostream& operator<<(ostream& os, const char *cstr) { os.write(cstr); return os;}

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
	virtual void flush() {
		m_os.flush();
	}
};

} // namespace lib

} // namespace ambulant


#endif // AMBULANT_LIB_AMSTREAM_H
