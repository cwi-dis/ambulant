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

#ifndef AMBULANT_LIB_WIN32_THREAD_H
#define AMBULANT_LIB_WIN32_THREAD_H

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

#include "ambulant/lib/thread.h"

#include "win32_error.h"


namespace ambulant {

namespace lib {

namespace win32 {

class AMBULANTAPI thread : public ambulant::lib::thread {
  public:
	thread();

	virtual ~thread();

	virtual bool start();
	virtual void stop();

	HANDLE get_stop_handle() const;
	HANDLE get_thread_handle() const;

	bool terminate();

	void set_winui_exit_listener(HWND hWnd, UINT winui_exit_msg);

	bool set_priority(int priority);

	bool relax(DWORD millis);
	bool is_running() const;

  protected:
	virtual unsigned long run() = 0;

	virtual void signal_exit_thread();

	bool exit_requested() const;

  private:
	static DWORD __stdcall threadproc(LPVOID pParam);

	HANDLE m_stop_event;
	HANDLE m_handle;
	DWORD m_id;
	DWORD m_parent_id;
	HWND m_wnd;
	UINT m_winui_exit_msg;
};

} // namespace win32

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_WIN32_THREAD_H
