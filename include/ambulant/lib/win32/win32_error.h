/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_LIB_WIN32_ERROR_H
#define AMBULANT_LIB_WIN32_ERROR_H

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

namespace ambulant {

namespace lib {

namespace win32 {

inline void win_report_error(const char *func, DWORD  err) {
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
	log_error_event("%s failed, Error 0x%x: %s", func, err, pMsgBuf);
	LocalFree(pMsgBuf);
}

inline void win_report_last_error(const char *func){
	win_report_error(func, GetLastError());
}

} // namespace win32

} // namespace lib

} // namespace ambulant 

#endif // AMBULANT_LIB_WIN32_ERROR_H
