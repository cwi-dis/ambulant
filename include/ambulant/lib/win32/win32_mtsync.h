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
  public:
	critical_section() { InitializeCriticalSection(&m_cs);}
	virtual ~critical_section() { DeleteCriticalSection(&m_cs);}

	void enter() { EnterCriticalSection(&m_cs);}
	void leave() { LeaveCriticalSection(&m_cs);}

  protected:
	CRITICAL_SECTION m_cs;
};

#ifdef WITH_WIN32_CONDITION_VARIABLE
class AMBULANTAPI critical_section_cv :public critical_section,  public ambulant::lib::base_critical_section_cv {
  public:
	critical_section_cv()
	:	critical_section()
	{
		InitializeConditionVariable(&m_cv);
	}
	~critical_section_cv() {
		CloseHandle(m_event);
	}

	void signal() {
		WakeConditionVariable(&m_cv);
	}
	bool wait(int microseconds) {
		DWORD timeout = microseconds < 0 ? INFINITE : microseconds/1000;
		bool rv = SleepConditionVariableCS(&m_cv, &m_cs, timeout);
		return rv;
	}
  private:
	CONDITION_VARIABLE m_cv;
};
#else
class AMBULANTAPI critical_section_cv :public critical_section,  public ambulant::lib::base_critical_section_cv {
  public:
	critical_section_cv()
	:	critical_section()
	{
		m_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
	~critical_section_cv() {
		CloseHandle(m_event);
	}

	void signal() {
		SetEvent(m_event);
	}
	bool wait(int microseconds = -1) {
		DWORD timeout = microseconds < 0 ? INFINITE : microseconds/1000;
		leave();
		bool rv = WaitForSingleObject(m_event, timeout) == WAIT_OBJECT_0;
		enter();
		if (rv) ResetEvent(m_event);
		return rv;
	}
  private:
	HANDLE m_event;
};
#endif
} // namespace win32

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_WIN32_MTSYNC_H
