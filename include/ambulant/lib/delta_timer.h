/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */
#ifndef AMBULANT_LIB_DELTA_TIMER_H
#define AMBULANT_LIB_DELTA_TIMER_H

// delta timer list
#include <list>

// fetch events
#include <queue>

// part of timeout event
#include "ambulant/lib/event.h"

// timers
#include "ambulant/lib/timer.h"

namespace ambulant {

namespace lib {

// A scheduler for timeout events
// Uses delta timer pattern

class delta_timer {
  public:
	typedef timer::time_type time_type;
	typedef std::pair<event*, time_type> timeout_event;
	
	delta_timer(timer *t);
	virtual ~delta_timer();

	// called periodically
	// the period defines timer resolution
	// fires ready events 
	void execute();
	
	// like execute but instead of executing events
	// returns the ready to fire events to the caller
	void execute(std::queue<event*>& queue);
	
	// Insert a timeout event
	virtual void insert(event *pe, time_type t);

	// debug output
	void write_trace();
	
  private:
	void fire_delta_events(time_type delta);
	void get_ready_delta_events(time_type delta, std::queue<event*>& queue);
	
	void decr(time_type& t, time_type dt) {
		if(dt>t) t = 0;
		else t-= dt;
	}
	
	// event list and its critical section
	std::list<timeout_event> m_events;
	
	// remember last time run
	time_type m_last_run;
	
	// timer
	timer *m_timer;
	};

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_DELTA_TIMER_H
