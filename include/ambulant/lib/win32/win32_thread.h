/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_LIB_WIN32_THREAD_H
#define AMBULANT_LIB_WIN32_THREAD_H

#include "ambulant/lib/thread.h"

#include "win32_error.h"

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

namespace ambulant {

namespace lib {

namespace win32 {

class thread : public ambulant::lib::thread {
  public:
	thread()
	:	m_stop_event(NULL),
		m_handle(NULL),
		m_id(0),
		m_parent_id(GetCurrentThreadId()),
		m_wnd(NULL) {
		m_stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);
		if(m_stop_event == NULL)
			win_report_last_error("CreateEvent()");
	}

	virtual ~thread() {
		if(m_handle != NULL) stop();
		if(m_stop_event) CloseHandle(m_stop_event);
	}

	virtual bool start() {
		if(m_handle != NULL || m_stop_event == NULL) 
			return false;
		ResetEvent(m_stop_event);
		m_handle = CreateThread(NULL, 0, &thread::threadproc, this, 0, &m_id);
		if(m_handle == NULL)
			win_report_last_error("CreateThread()");
		Sleep(0);
		return m_handle != NULL;
	}

	virtual void stop(){
		if(m_handle != NULL) {
			SetEvent(m_stop_event);
			WaitForSingleObject(m_handle, INFINITE);
			CloseHandle(m_handle);
			m_handle = NULL;
		}
	}
	
	HANDLE get_stop_handle() const { return m_stop_event;}
	HANDLE get_thread_handle() const { return m_handle;}

	bool terminate()
		{return TerminateThread(m_handle, 1) == TRUE;}
	
	void set_winui_exit_listener(HWND hWnd, UINT winui_exit_msg) 
		{ m_wnd = hWnd; m_winui_exit_msg = winui_exit_msg;}

	bool set_priority(int priority) {
		if(m_handle == NULL) return false;
		if(::SetThreadPriority(m_handle, priority) == FALSE)
			{
			win_report_last_error("SetThreadPriority()");
			return false;
			}
		return true;
	}
	
	bool relax(DWORD millis) {
		return WaitForSingleObject(get_stop_handle(), millis) != WAIT_OBJECT_0;
	}

	bool is_running() const {
		if(m_handle == NULL) return false;
		return (WaitForSingleObject(m_handle, 0) != WAIT_OBJECT_0) &&
			(WaitForSingleObject(m_stop_event, 0) != WAIT_OBJECT_0);
	}
		
  protected:
	virtual unsigned long run() = 0;
	
	virtual void signal_exit_thread(){
		SetEvent(get_stop_handle());
		if(m_wnd != NULL)
			PostMessage(m_wnd, m_winui_exit_msg, 0, 0);
	}
	
	bool exit_requested() const {
		return WaitForSingleObject(get_stop_handle(), 0) == WAIT_OBJECT_0; 
	}

  private:
	static DWORD __stdcall threadproc(LPVOID pParam) {
		thread* p = static_cast<thread*>(pParam);
		unsigned long dw = p->run();
		ExitThread(dw);
		return dw;
	}

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
