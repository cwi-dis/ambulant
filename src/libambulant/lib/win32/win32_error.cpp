
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/win32/win32_error.h"

#include "ambulant/lib/logger.h"

#include <windows.h>

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
	lib::logger::get_logger()->error("%s failed, Error 0x%x: %s", func, err, pMsgBuf);
	LocalFree(pMsgBuf);
}

void lib::win32::win_report_last_error(const char *func){
	win_report_error(func, GetLastError());
}
