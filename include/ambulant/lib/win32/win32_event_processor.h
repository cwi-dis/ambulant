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

#ifndef _QUEUE_
#include <queue>
#endif

#ifndef AMBULANT_LIB_EVENT_H
#include "ambulant/lib/event.h"
#endif

#ifndef AMBULANT_LIB_LOGGER_H
#include "ambulant/lib/logger.h"
#endif

#ifndef AMBULANT_LIB_DELTA_TIMER_H
#include "ambulant/lib/delta_timer.h"
#endif

#ifndef AMBULANT_LIB_WIN32_THREAD_H
#include "win32_thread.h"
#endif

#ifndef AMBULANT_LIB_WIN32_MTSYNC_H
#include "win32_mtsync.h"
#endif

#ifndef AMBULANT_LIB_WIN32_TIMER_H
#include "win32_timer.h"
#endif

#ifndef _INC_WINDOWS
#include <windows.h>
#endif


namespace ambulant {

namespace lib {

namespace win32 {

class event_processor : public thread {
  public:
	typedef timer::time_type time_type;
	
	event_processor() 
	:   m_wait_event(0),
		m_delta_timer(new os_timer()) {
		m_wait_event = CreateEvent(NULL, TRUE, FALSE, NULL);
		if(m_wait_event == 0)
			win_report_last_error("CreateEvent()");	
	}
	
	~event_processor() {
		// close wait event
		if(m_wait_event) 
			CloseHandle(m_wait_event);
	}
  
	void add_event(event *pe, time_type t) {
		m_delta_timer_cs.enter();
		m_delta_timer.insert(pe, t);
		wakeup();
 		m_delta_timer_cs.leave();
	}
  
  protected:
	virtual DWORD run() {
		log_trace_event("event_processor started");
		while(!exit_requested()) {			
			std::queue<event*> queue;
			m_delta_timer.execute(queue);
			while(!queue.empty()) {
				event *e = queue.front();
				queue.pop();
				e->fire();
				delete e;
			}
			wait_event();
		}
		log_trace_event("event_processor stopped");
		return 0;
	}
	
  private:
	void wait_event() {
		WaitForSingleObject(m_wait_event, 100);
	}
	
	void wakeup() {
		SetEvent(m_wait_event);
	}
		
	//std::queue<event*> m_queue;
	lib::delta_timer m_delta_timer;
	critical_section m_delta_timer_cs;
	
	HANDLE m_wait_event;
};


} // namespace win32

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_WIN32_EVENT_PROCESSOR_H
