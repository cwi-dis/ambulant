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

#ifndef AMBULANT_LIB_TEXTPTR_H
#define AMBULANT_LIB_TEXTPTR_H

#include "ambulant/config/config.h"

#ifdef AMBULANT_PLATFORM_WIN32
	#ifndef _INC_WINDOWS
	#include <windows.h>
	#endif
#else
	#include <stdio.h>
	#include <wchar.h>
#endif

#ifndef text_str
	#ifdef UNICODE
	#define text_str(quote) L##quote
	#else
	#define text_str(quote) quote
	#endif
#endif

namespace ambulant {

namespace lib {

// This class is a two writing_modes converter
// between UNICODE an MB strings.

class textptr {
  public:
	typedef wchar_t* wchar_ptr;
	typedef const wchar_t* const_wchar_ptr;

	typedef char* char_ptr;
	typedef const char* const_char_ptr;

	textptr(const char *pb)
	:	m_pcb(pb), m_pcw(NULL), m_pb(NULL), m_pw(NULL) {}

	textptr(const wchar_t *pw)
	:	m_pcb(NULL), m_pcw(pw), m_pb(NULL), m_pw(NULL) {}

	~textptr() {
		if(m_pw != NULL) delete[] m_pw;
		if(m_pb != NULL) delete[] m_pb;
	}

	wchar_ptr wstr() {
		if(m_pcw != NULL) return const_cast<wchar_ptr>(m_pcw);
		if(m_pw != NULL) return m_pw;
		if(m_pcb == NULL) return NULL;
		size_t n = strlen(m_pcb) + 1;
		m_pw = new wchar_t[n];
#ifdef AMBULANT_PLATFORM_WIN32
		MultiByteToWideChar(CP_UTF8, 0, m_pcb, -1, m_pw, n);
#else
		mbstowcs(m_pw, m_pcb, n);
#endif
		return m_pw;
	}
	const_wchar_ptr c_wstr() { return wstr();}

	char_ptr str() {
		if(m_pcb != NULL) return const_cast<char_ptr>(m_pcb);
		if(m_pb != NULL) return m_pb;
		if(m_pcw == NULL) return NULL;
		std::ptrdiff_t n = wcslen(m_pcw)*2+1; // Two times wide string size should be enough for mb
		m_pb = new char[n];
#ifdef AMBULANT_PLATFORM_WIN32
		WideCharToMultiByte(CP_UTF8, 0, m_pcw, -1, m_pb, static_cast<int>(n), NULL, NULL);
#else
		wcstombs(m_pb, m_pcw, n);

#endif
		return m_pb;
	}
	const_char_ptr c_str() { return str();}

	operator wchar_ptr() { return wstr();}
	operator const_wchar_ptr() { return c_wstr();}

	operator char_ptr() { return str();}
	operator const_char_ptr() { return c_str();}

  private:
	const_char_ptr m_pcb;
	const_wchar_ptr m_pcw;
	char_ptr m_pb;
	wchar_ptr m_pw;
};


} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_TEXTPTR_H
