

/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/win32/win32_thread.h"

#include "ambulant/lib/thread.h"

#include "ambulant/lib/win32/win32_error.h"

using namespace ambulant;

lib::win32::thread::thread()
:	m_stop_event(NULL),
	m_handle(NULL),
	m_id(0),
	m_parent_id(GetCurrentThreadId()),
	m_wnd(NULL) {
	m_stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(m_stop_event == NULL)
		win_report_last_error("CreateEvent()");
}

lib::win32::thread::~thread() {
	if(m_handle != NULL) stop();
	if(m_stop_event) CloseHandle(m_stop_event);
}

bool lib::win32::thread::start() {
	if(m_handle != NULL || m_stop_event == NULL) 
		return false;
	ResetEvent(m_stop_event);
	m_handle = CreateThread(NULL, 0, &thread::threadproc, this, 0, &m_id);
	if(m_handle == NULL)
		win_report_last_error("CreateThread()");
	Sleep(0);
	return m_handle != NULL;
}

void lib::win32::thread::stop(){
	if(m_handle != NULL) {
		SetEvent(m_stop_event);
		WaitForSingleObject(m_handle, INFINITE);
		CloseHandle(m_handle);
		m_handle = NULL;
	}
}
	
HANDLE lib::win32::thread::get_stop_handle() const { 
	return m_stop_event;
}

HANDLE lib::win32::thread::get_thread_handle() const { 
	return m_handle;
}

bool lib::win32::thread::terminate() {
	return TerminateThread(m_handle, 1) == TRUE;
}
	
void lib::win32::thread::set_winui_exit_listener(HWND hWnd, UINT winui_exit_msg) { 
	m_wnd = hWnd; 
	m_winui_exit_msg = winui_exit_msg;
}

bool lib::win32::thread::set_priority(int priority) {
	if(m_handle == NULL) return false;
	if(::SetThreadPriority(m_handle, priority) == FALSE)
		{
		win_report_last_error("SetThreadPriority()");
		return false;
		}
	return true;
}
	
bool lib::win32::thread::relax(DWORD millis) {
	return WaitForSingleObject(get_stop_handle(), millis) != WAIT_OBJECT_0;
}

bool lib::win32::thread::is_running() const {
	if(m_handle == NULL) return false;
	return (WaitForSingleObject(m_handle, 0) != WAIT_OBJECT_0) &&
		(WaitForSingleObject(m_stop_event, 0) != WAIT_OBJECT_0);
}

void lib::win32::thread::signal_exit_thread(){
	SetEvent(get_stop_handle());
	if(m_wnd != NULL)
		PostMessage(m_wnd, m_winui_exit_msg, 0, 0);
}
	
bool lib::win32::thread::exit_requested() const {
	return WaitForSingleObject(get_stop_handle(), 0) == WAIT_OBJECT_0; 
}

//static 
DWORD __stdcall lib::win32::thread::threadproc(LPVOID pParam) {
	thread* p = static_cast<thread*>(pParam);
	unsigned long dw = p->run();
	ExitThread(dw);
	return dw;
}

