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

// Ambulant standard base (ASB) compatibility implementations

#include <windows.h>

#include "ambulant/lib/win32/win32_asb.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/win32/win32_error.h"

#include <string>
#include <cstring>

using namespace ambulant;

void lib::win32::sleep(unsigned long secs) {
	Sleep(1000*secs);
}

void lib::win32::sleep_msec(unsigned long msecs) {
	Sleep(msecs);
}

std::basic_string<text_char> lib::win32::getcwd() {
#ifndef AMBULANT_PLATFORM_WIN32_WCE
	text_char buf[MAX_PATH];
	text_char *pFilePart = 0;	
	GetFullPathName(text_str("."), MAX_PATH, buf, &pFilePart);
	return buf;
#endif
	return text_str(".");
}

std::basic_string<text_char> lib::win32::resolve_path(const text_char *s) {
#ifndef AMBULANT_PLATFORM_WIN32_WCE
	text_char buf[MAX_PATH];
	text_char *pFilePart = 0;	
	GetFullPathName(s, MAX_PATH, buf, &pFilePart);
	return buf;
#endif
	return s;
}

std::basic_string<text_char> lib::win32::get_module_filename() {
	text_char buf[MAX_PATH];
	GetModuleFileName(NULL, buf, MAX_PATH);
	return buf;
}

std::basic_string<text_char> lib::win32::get_module_dir() {
	text_char buf[MAX_PATH];
	GetModuleFileName(NULL, buf, MAX_PATH);
#ifdef AMBULANT_PLATFORM_WIN32_WCE
	wchar_t *p = wcsrchr(buf, '\\');
#else
	char *p = strrchr(buf, '\\');
#endif
	if (p) p[1] = '\0';
	return buf;
}

#ifndef AMBULANT_PLATFORM_WIN32_WCE
// WINCE should provide its own version
void lib::win32::show_message(int level, const char *message) {
	unsigned int type = MB_OK;
	if (level == lib::logger::LEVEL_WARN) type |= MB_ICONWARNING;
	if (level == lib::logger::LEVEL_ERROR) type |= MB_ICONERROR;
	if (level == lib::logger::LEVEL_FATAL) type |= MB_ICONERROR;
	MessageBox(NULL, textptr(message), text_str("AmbulantPlayer"), type);
}
#endif

bool lib::win32::file_exists(const std::string& fn) {
	WIN32_FIND_DATA fd;
	memset(&fd, 0, sizeof(WIN32_FIND_DATA));
	bool exists = false;
	textptr tp(fn.c_str());
	HANDLE hFind = FindFirstFile(tp, &fd); 
	if(hFind != INVALID_HANDLE_VALUE){
		FindClose(hFind);
		hFind = INVALID_HANDLE_VALUE;
		exists = true;
	} else  {
		DWORD dw = GetLastError();

		if(dw != ERROR_FILE_NOT_FOUND)
			/*win_report_error("FindFirstFile()", dw)*/;
	}
	return exists;
}
