/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_LIB_UNIX_EVENT_PROCESSOR_H
#define AMBULANT_LIB_UNIX_EVENT_PROCESSOR_H

#include "ambulant/lib/event_processor.h"

#include "unix_thread.h"
#include "unix_mtsync.h"
#include "unix_timer.h"

namespace ambulant {

namespace lib {

namespace unix {

typedef ambulant::lib::unix::critical_section unix_cs;

class event_processor : 
  public ambulant::lib::abstract_event_processor,
  public ambulant::lib::unix::thread {

  public:
	event_processor() 
	:   abstract_event_processor(new os_timer(), new unix_cs()) {
	}
	
	~event_processor() {
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
	void wait_event() {
		m_event_sema.down();
	}
	
	void wakeup() {
		m_event_sema.up();
	}
		
	counting_semaphore m_event_sema;
	
};


} // namespace unix

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_UNIX_EVENT_PROCESSOR_H
