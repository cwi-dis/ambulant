
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_LIB_WIN32_MTSYNC_H
#define AMBULANT_LIB_WIN32_MTSYNC_H

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

namespace ambulant {

namespace lib {

namespace win32 {

class critical_section {
  public:
	critical_section() { InitializeCriticalSection(&m_cs);}
	~critical_section() { DeleteCriticalSection(&m_cs);}

	void enter() { EnterCriticalSection(&m_cs);}
	void leave() { LeaveCriticalSection(&m_cs);}

  private:
	CRITICAL_SECTION m_cs;
};

} // namespace win32

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_WIN32_MTSYNC_H
