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

#ifndef AMBULANT_LIB_WIN32_FSTREAM_H
#define AMBULANT_LIB_WIN32_FSTREAM_H

#ifndef _INC_WINDOWS
#include <windows.h>
#endif // _INC_WINDOWS

#include <string.h>

#include "ambulant/config/config.h"
#include "ambulant/lib/amstream.h"

namespace ambulant {

namespace lib {

namespace win32 {

class AMBULANTAPI fstream : public lib::istream, public lib::ostream {
  public:
	fstream();
	~fstream();

	bool open(const std::basic_string<char>& url);
	bool open(const text_char *filename);
	bool open_for_writing(const text_char *filename);
	bool is_open() const { return m_hf != INVALID_HANDLE_VALUE;}
	void close();

	int read(unsigned char *buffer, int nbytes);
	int read();

	unsigned long gptr() const { return m_gptr;}
	unsigned long pptr() const { return m_pptr;}

	unsigned long get_size();
	bool seek(unsigned long pos);

	int write(const unsigned char *buffer, int nbytes);

	int write(const char *cstr);

	virtual void flush() {}

  private:
	HANDLE m_hf;
	unsigned long m_gptr;
	unsigned long m_pptr;
};

} // namespace win32

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_WIN32_FSTREAM_H
