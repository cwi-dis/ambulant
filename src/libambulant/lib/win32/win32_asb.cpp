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

// Ambulant standard base (ASB) compatibility implementations

#include <windows.h>

#include "ambulant/lib/win32/win32_asb.h"
#include "ambulant/lib/textptr.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/win32/win32_error.h"
#ifdef _UNICODE
#define TSTR_TO_STR(s) ambulant::lib::textptr(s).c_str()
#else
#define TSTR_TO_STR(s) (s)
#endif

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
	text_char buf[MAX_PATH];
	text_char *pFilePart = 0;
	GetFullPathName(text_str("."), MAX_PATH, buf, &pFilePart);
	return buf;
}

std::basic_string<text_char> lib::win32::resolve_path(const text_char *s) {
	text_char buf[MAX_PATH];
	text_char *pFilePart = 0;
	GetFullPathName(s, MAX_PATH, buf, &pFilePart);
	return buf;
}

std::basic_string<text_char> lib::win32::get_module_filename() {
	text_char buf[MAX_PATH];
	GetModuleFileName(NULL, buf, MAX_PATH);
	return buf;
}

std::string lib::win32::get_module_dir() {
	text_char buf[MAX_PATH];
#ifdef USE_MAIN_EXECUTABLE_LOCATION
	GetModuleFileName(NULL, buf, MAX_PATH);
#else
	HMODULE hm = NULL;
	GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCTSTR)&lib::win32::get_module_dir,
		&hm);
	GetModuleFileName(hm, buf, MAX_PATH);
#endif
#ifdef _UNICODE
	wchar_t *p = wcsrchr(buf, '\\');
#else
	char *p = strrchr(buf, '\\');
#endif
	if (p) p[1] = '\0';
	return std::string(TSTR_TO_STR(buf));
}

void lib::win32::show_message(int level, const char *message) {
	unsigned int type = MB_OK;
	if (level == lib::logger::LEVEL_WARN) type |= MB_ICONWARNING;
	if (level == lib::logger::LEVEL_ERROR) type |= MB_ICONERROR;
	if (level == lib::logger::LEVEL_FATAL) type |= MB_ICONERROR;
	MessageBox(NULL, textptr(message), text_str("AmbulantPlayer"), type);
}

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
	}
	return exists;
}
