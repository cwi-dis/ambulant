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

#ifndef AMBULANT_LIB_EVENT_H
#include "event.h"
#endif

#ifndef AMBULANT_LIB_TIMER_H
#include "timer.h"
#endif

// tracing
// delta_timer::write_trace
#ifndef AMBULANT_LIB_LOGGER_H
#include "logger.h"
#endif
#include <sstream>


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


inline delta_timer::delta_timer(timer *t)
:	m_timer(t), m_last_run(t->elapsed()) {}
	
inline delta_timer::~delta_timer() {
	std::list<timeout_event>::iterator it;
	for(it = m_events.begin(); it != m_events.end(); it++)
		delete (*it).first;
}

// called periodically
// fires ready events	
inline void delta_timer::execute() {
	time_type now = m_timer->elapsed();
	time_type delta = now - m_last_run;
	m_last_run = now;
	fire_delta_events(delta);
}
	
// called periodically
// returns the events that are ready to fire	
inline void delta_timer::execute(std::queue<event*>& queue) {
	time_type now = m_timer->elapsed();
	time_type delta = now - m_last_run;
	m_last_run = now;
	get_ready_delta_events(delta, queue);
}

inline void delta_timer::insert(event *pe, time_type t) {
	if(m_events.size() == 0){
		m_events.push_back(timeout_event(pe, t));
	} else {
		// find correct pos (do: pe->incr(-(*i).get_time() as we traverse the list)
		std::list<timeout_event>::iterator it;
		for(it = m_events.begin();it!=m_events.end();it++) {
			if(t < (*it).second) 
				break;
			decr(t, (*it).second); 
		}
		// insert entry
		if(it != m_events.end()) {
			decr((*it).second, t);
			m_events.insert(it, timeout_event(pe, t));
		} else {
			m_events.push_back(timeout_event(pe, t));
		}
	}
	// debug
	write_trace();
}

inline void delta_timer::fire_delta_events(time_type delta) {
	while(m_events.size()>0 && m_events.front().second <= delta) {
		decr(delta, m_events.front().second);
		m_events.front().first->fire();
		delete m_events.front().first;
		m_events.erase(m_events.begin());
	}
	if(m_events.size()>0)
		decr(m_events.front().second, delta);
}
	
inline void delta_timer::get_ready_delta_events(time_type delta, std::queue<event*>& queue) {
	while(m_events.size()>0 && m_events.front().second <= delta) {
		decr(delta, m_events.front().second);
		queue.push(m_events.front().first);
		m_events.erase(m_events.begin());
	}
	if(m_events.size()>0) 
		decr(m_events.front().second, delta); 
			
	// debug
	if(!queue.empty()) write_trace();
}
	
inline void delta_timer::write_trace() {
	std::ostringstream os;
	os << "delta_timer (";
	std::list<timeout_event>::iterator it = m_events.begin();
	if(it != m_events.end()) {
		os << (*it).second;
		it++;
	}
	for(; it != m_events.end(); it++)
		os << ", " << (*it).second;
	os << ")";
	log_trace_event(os.str());
}


} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_DELTA_TIMER_H
