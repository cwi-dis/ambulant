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

#include "ambulant/lib/win32/win32_memfile.h"
#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/logger.h"
#include <wininet.h>

#pragma comment (lib,"wininet.lib")

using namespace ambulant;

lib::win32::memfile::memfile(const net::url& u)
:	m_url(u), m_gptr(0) {
}
	
lib::win32::memfile::~memfile() {
}

// static 
bool lib::win32::memfile::exists(const net::url& u) {
	if(!u.is_local_file()) return true;
	std::string file = u.get_file();
	textptr tp(file.c_str());
	HANDLE hf = CreateFile(tp,  
		GENERIC_READ,  
		FILE_SHARE_READ,  // 0 = not shared or FILE_SHARE_READ  
		0,  // lpSecurityAttributes 
		OPEN_EXISTING,  
		FILE_ATTRIBUTE_READONLY,  
		NULL); 
	if(hf == INVALID_HANDLE_VALUE) {
		lib::logger::get_logger()->show("Failed to open file %s", 
			u.get_file().c_str());
		return false;
	}
	CloseHandle(hf);
	return true;
}

bool lib::win32::memfile::read() {
	return m_url.is_local_file()?read_local(m_url.get_file()):
		read_remote(m_url.get_url());	
}

bool lib::win32::memfile::read_local(const std::string& fn) {
	textptr tp(fn.c_str());
	HANDLE hf = CreateFile(tp,  
		GENERIC_READ,  
		FILE_SHARE_READ,  // 0 = not shared or FILE_SHARE_READ  
		0,  // lpSecurityAttributes 
		OPEN_EXISTING,  
		FILE_ATTRIBUTE_READONLY,  
		NULL); 
	if(hf == INVALID_HANDLE_VALUE) {
		lib::logger::get_logger()->show("Failed to open file %s", fn.c_str());
		return false;
	}
	const int buf_size = 1024;
	byte *buf = new byte[buf_size];
	DWORD nread = 0;
	while(ReadFile(hf, buf, buf_size, &nread, 0) && nread>0){
		m_buffer.append(buf, nread);
	}
	delete[] buf;
	m_gptr = 0;
	CloseHandle(hf);
	return true;
}

bool lib::win32::memfile::read_remote(const std::string& urlstr) {
	HINTERNET hinet = InternetOpen(text_str("AmbulantPlayer"), 
		INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if(!hinet) {
		win_report_last_error("InternetOpen()");
		return false;
	}
	
	text_char urlbuf[512];
	DWORD nch = 512;
	
	textptr tp(urlstr.c_str());
	BOOL bres = InternetCanonicalizeUrl(tp, urlbuf , &nch, ICU_BROWSER_MODE);
	if(!bres) {
		win_report_last_error("InternetCanonicalizeUrl()");
		InternetCloseHandle(hinet); 
		return false;
	}
	
	HINTERNET hf = InternetOpenUrl(hinet, urlbuf,  NULL, 0, INTERNET_FLAG_RAW_DATA, 0);
	if(!hf) {
		win_report_last_error("InternetOpenUrl()");
		InternetCloseHandle(hinet); 
		return false;
	}
	const int buf_size = 1024;
	byte *buf = new byte[buf_size];
	DWORD nread = 0;
	bool succeeded = true;
	do {
		bres = InternetReadFile(hf, buf, buf_size, &nread);
		if(!bres) {
			win_report_last_error("InternetReadFile()");
			succeeded = false;
			break;
		} else {
			m_buffer.append(buf, nread);
		}
	} while(nread > 0);
	delete[] buf;	
	InternetCloseHandle(hf); 
	InternetCloseHandle(hinet); 
	return succeeded;
}


void lib::win32::memfile::throw_range_error() {
#ifndef AMBULANT_PLATFORM_WIN32_WCE_3
	throw std::range_error("index out of range");
#else
	assert(false);
#endif
}

std::string lib::win32::memfile::repr() {
	std::string s("memfile[");
	//s += (!m_url.empty()?m_url:"NULL");
	s += "]";
	return s;
};
