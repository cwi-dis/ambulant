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
#include "ambulant/lib/unix/unix_thread.h"
#include "ambulant/lib/mtsync.h"

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
	event_processor(abstract_timer *t);
	~event_processor();
  
  protected:
	virtual unsigned long run();
	
  private:
	void wait_event();
	void wakeup();
	pthread_cond_t m_queue_condition;
	pthread_mutex_t m_queue_mutex;	
};


} // namespace unix

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_UNIX_EVENT_PROCESSOR_H
