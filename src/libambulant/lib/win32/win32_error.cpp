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

#include <windows.h>

#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/textptr.h"


using namespace ambulant;

void lib::win32::win_report_error(const char *func, unsigned long  err) {
	char* pMsgBuf;
	::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &pMsgBuf,
		0,
		NULL
		);
	textptr tp(pMsgBuf);
	lib::logger::get_logger()->error("%s failed, Error 0x%x: %s", func, err, tp.c_str());
	LocalFree(pMsgBuf);
}

void lib::win32::win_report_last_error(const char *func){
	win_report_error(func, GetLastError());
}

void lib::win32::win_show_error(const char *func, unsigned long  err) {
	char* pMsgBuf;
	::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &pMsgBuf,
		0,
		NULL
		);
	textptr tp(pMsgBuf);
	lib::logger::get_logger()->show("%s failed, Error 0x%x: %s", func, err, tp.c_str());
	LocalFree(pMsgBuf);
}


void lib::win32::win_show_last_error(const char *func){
	win_report_error(func, GetLastError());
}

void lib::win32::win_trace_error(const char *func, unsigned long  err) {
	char* pMsgBuf;
	::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &pMsgBuf,
		0,
		NULL
		);
	textptr tp(pMsgBuf);
	lib::logger::get_logger()->trace("%s failed, Error 0x%x: %s", func, err, tp.c_str());
	LocalFree(pMsgBuf);
}

void lib::win32::win_trace_last_error(const char *func){
	win_report_error(func, GetLastError());
}

