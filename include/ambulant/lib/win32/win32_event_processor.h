/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_LIB_WIN32_EVENT_PROCESSOR_H
#define AMBULANT_LIB_WIN32_EVENT_PROCESSOR_H

#include "ambulant/lib/event_processor.h"

#include "win32_thread.h"
#include "win32_error.h"
#include "win32_timer.h"
#include "win32_mtsync.h"

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

namespace ambulant {

namespace lib {

namespace win32 {

typedef ambulant::lib::win32::critical_section win32_cs;

class event_processor : 
  public ambulant::lib::abstract_event_processor,
  public ambulant::lib::win32::thread {
  
  public:
	event_processor() 
	:   abstract_event_processor(new os_timer(), new win32_cs()),
		m_wait_event(0) {
		m_wait_event = CreateEvent(NULL, TRUE, FALSE, NULL);
		if(m_wait_event == 0)
			win_report_last_error("CreateEvent()");	
	}
	
	~event_processor() {
		if(m_wait_event) 
			CloseHandle(m_wait_event);
	}
    
  protected:
	virtual unsigned long run() {
		log_trace_event("event_processor started");
		while(!exit_requested()) {	
			serve_events();		
			wait_event();
		}
		log_trace_event("event_processor stopped");
		return 0;
	}
	
  private:
	// wait until some thread calls wakeup
	void wait_event() {
		WaitForSingleObject(m_wait_event, 100);
	}
	
	// wakes up this thread
	void wakeup() {
		SetEvent(m_wait_event);
	}
	HANDLE m_wait_event;
};


} // namespace win32

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_WIN32_EVENT_PROCESSOR_H
