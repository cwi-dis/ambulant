/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2005 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_LIB_WIN32_MTSYNC_H
#define AMBULANT_LIB_WIN32_MTSYNC_H

#ifndef _INC_WINDOWS

#include <windows.h>
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#endif // _INC_WINDOWS

#include "ambulant/config/config.h"

#include "ambulant/lib/abstract_mtsync.h"

namespace ambulant {

namespace lib {

namespace win32 {

class AMBULANTAPI critical_section : public ambulant::lib::base_critical_section {
	friend class condition;
  public:
	critical_section() { InitializeCriticalSection(&m_cs);}
	~critical_section() { DeleteCriticalSection(&m_cs);}

	void enter() { EnterCriticalSection(&m_cs);}
	void leave() { LeaveCriticalSection(&m_cs);}

  private:
	CRITICAL_SECTION m_cs;
};

class AMBULANTAPI condition : public ambulant::lib::base_condition {
  public:
	  condition() { m_event = CreateEvent(NULL, TRUE, FALSE, NULL);}
	  ~condition() { CloseHandle(m_event); }
	
	  void signal() { SetEvent(m_event); }
	  void signal_all() { abort(); }
	  bool wait(int microseconds, critical_section &cs) {
		  DWORD timeout = microseconds < 0 ? INFINITE : microseconds/1000;
		  bool rv = WaitForSingleObject(m_event, timeout) == WAIT_OBJECT_0;
		  if (rv) {
			  EnterCriticalSection(&cs.m_cs);
			  ResetEvent(m_event);
		  }
		  return rv;
	  }
  private:
	HANDLE m_event;
};
} // namespace win32

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_WIN32_MTSYNC_H
