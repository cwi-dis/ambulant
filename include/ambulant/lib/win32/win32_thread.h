/* 
 * @$Id$ 
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
