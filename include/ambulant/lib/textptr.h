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

/* 
 * @$Id$ 
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

namespace ambulant {

namespace lib {

// This class is a two directions converter 
// between UNICODE an MB strings.

class textptr {
  public:
	typedef wchar_t* wchar_ptr;
	typedef const wchar_t* const_wchar_ptr;
	
	typedef char* char_ptr;
	typedef const char* const_char_ptr;

	textptr(const char *pb) 
	:	m_pcb(pb), m_pcw(NULL), m_pb(NULL), m_pw(NULL), m_length(-1) {}

	textptr(const wchar_t *pw) 
	:	m_pcb(NULL), m_pcw(pw), m_pb(NULL), m_pw(NULL), m_length(-1) {}

	~textptr() {
		if(m_pw != NULL) delete[] m_pw;
		if(m_pb != NULL) delete[] m_pb;
	}
	
	wchar_ptr wstr() {
		if(m_pcw != NULL) return const_cast<wchar_ptr>(m_pcw);
		if(m_pw != NULL) return m_pw;
		if(m_pcb == NULL) return NULL;
		m_length = strlen(m_pcb);
		int n = (int)m_length + 1;
		m_pw = new wchar_t[n];
#ifdef AMBULANT_PLATFORM_WIN32
		MultiByteToWideChar(CP_ACP, 0, m_pcb, n, m_pw, n);
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
		m_length = wcslen(m_pcw);
		int n = (int)m_length + 1;
		m_pb = new char[n];
#ifdef AMBULANT_PLATFORM_WIN32
		WideCharToMultiByte(CP_ACP, 0, m_pcw, n, m_pb, n, NULL, NULL);
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

	size_t length() {
		if(m_length>=0) return m_length;
		const_char_ptr pb = (m_pcb!=NULL)?m_pcb:m_pb;
		if(pb != NULL) 
			return (m_length = strlen(pb));
		const_wchar_ptr pw = (m_pcw!=NULL)?m_pcw:m_pw;
		if(pw != NULL) 
			return (m_length = wcslen(pw));
		return (m_length = 0);
	}

  private:
	const_char_ptr m_pcb;
	const_wchar_ptr m_pcw;
	char_ptr m_pb;
	wchar_ptr m_pw;
	size_t m_length;
};


} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_TEXTPTR_H
