/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_LIB_EVENT_PROCESSOR_H
#define AMBULANT_LIB_EVENT_PROCESSOR_H

#include "ambulant/lib/event.h"
#include "ambulant/lib/timer.h"

namespace ambulant {

namespace lib {

class event_processor {
  public:
	typedef timer::time_type time_type;
	enum event_priority {low, med, high};
	
	virtual ~event_processor() {}
	
	// schedule an event to fire at time t at the provided priority
	virtual void add_event(event *pe, time_type t, event_priority priority = low) = 0;
	
	// serves waiting events 
	virtual void serve_events() = 0;
};

} // namespace lib

} // namespace ambulant



/////////////////////////////////////////////////
// abstract_event_processor

#include <queue>

#include "ambulant/lib/logger.h"
#include "ambulant/lib/delta_timer.h"
#include "ambulant/lib/mtsync.h"

namespace ambulant {

namespace lib {

class abstract_event_processor : public event_processor {
  public:
	abstract_event_processor(timer *t, critical_section *pcs) 
	:	m_timer(t),
		m_high_delta_timer(t), 
		m_med_delta_timer(t), 
		m_low_delta_timer(t), 
		m_delta_timer_cs(pcs) {}
	
	~abstract_event_processor() {
		delete m_timer;
		delete m_delta_timer_cs;
	}
	
	void add_event(event *pe, time_type t, event_priority priority = low) {
		m_delta_timer_cs->enter();
		switch(priority) {
			case high: 
				m_high_delta_timer.insert(pe, t);
				break;
			case med: 
				m_med_delta_timer.insert(pe, t);
				break;
			case low: 
				m_low_delta_timer.insert(pe, t);
				break;
		}
		wakeup();
 		m_delta_timer_cs->leave();
	}
	
	// serves all events of high priority (including those inserted by firing events)
	// then med and finally low. Note that medium and low priority can cause new
	// events of higher priority to be scheduled, these should be handled first
	void serve_events() {
		int another_loop = 1;
		
		while (another_loop) {
			another_loop = 0;
			// First serve all high priority events
			while (serve_events_for(m_high_delta_timer))
				another_loop = 1;
			// After that, serve one medium priority event, if needed
			// XXXX Note this is incorrect, really: we only want to serve
			// one event but actually we serve all medium priority events
			// that are runnable. It remains to be seen whether this is
			// a problem. Fixing it would require keeping the queues
			// in the object in stead of as local variables in serve_events_for().
			if (serve_events_for(m_med_delta_timer))
				another_loop = 1;
			// if there were no medium prio events we serve a lo-prio event
			else if (serve_events_for(m_low_delta_timer))
				another_loop = 1;
		}
	}

	bool serve_events_for(delta_timer& dt) {
		std::queue<event*> queue;
		m_delta_timer_cs->enter();
		dt.execute(queue);
 		m_delta_timer_cs->leave();
		bool repeat = !queue.empty();
		while(!queue.empty()) {
			event *e = queue.front();
			queue.pop();
			e->fire();
			delete e;
		}
		return repeat; 
	}

  protected:
	// called by add_event
	// wakes up thread executing serve_events
	virtual void wakeup() = 0;
	
	// wait until some thread calls wakeup
	virtual void wait_event() = 0;
	
	// the timer for this processor
	timer *m_timer;
	
	// high priority delta timer
	delta_timer m_high_delta_timer;
	
	// medium priority delta timer
	delta_timer m_med_delta_timer;
	
	// low priority delta timer
	delta_timer m_low_delta_timer;
	
	// protects delta timer lists
	critical_section *m_delta_timer_cs;  
};

// Machine-dependent factory function
event_processor *event_processor_factory();

} // namespace lib

} // namespace ambulant


#endif // AMBULANT_LIB_EVENT_PROCESSOR_H
