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

template <typename time_type = unsigned long>
class delta_timer {
  public:
	typedef time_type self_time_type;
	typedef timer<self_time_type> self_timer;
	typedef timeout_event<self_time_type> self_timeout_event;
	
	delta_timer(self_timer *t);
	virtual ~delta_timer();

	// called periodically
	// the period defines timer resolution
	// fires ready events 
	void execute();
	
	// like execute but instead of executing events
	// returns the ready to fire events to the caller
	void execute(std::queue<event*>& queue);
	
	// Insert a timeout event
	virtual void insert(self_timeout_event *pe);

	// debug output
	void write_trace();
	
  private:
	void fire_delta_events(self_time_type delta);
	void get_ready_delta_events(self_time_type delta, std::queue<event*>& queue);
	
	// event list type
	typedef std::list<self_timeout_event*> event_list;
	
	// event list and its critical section
	event_list m_events;
	
	// remember last time run
	self_time_type m_last_run;
	
	// timer
	self_timer *m_timer;
	};


template <typename time_type>
inline delta_timer<time_type>::delta_timer(self_timer *t)
:	m_timer(t), m_last_run(t->elapsed()) {
	}
	
template <typename time_type>
inline delta_timer<time_type>::~delta_timer() {
	event_list::iterator it;
	for(it = m_events.begin(); it != m_events.end(); it++)
		delete (*it);
	delete m_timer;
}

// called periodically
// fires ready events	
template <typename time_type>
inline void delta_timer<time_type>::execute() {
	time_type now = m_timer->elapsed();
	time_type delta = now - m_last_run;
	m_last_run = now;
	fire_delta_events(delta);
}
	
// called periodically
// returns the events that are ready to fire	
template <typename time_type>
inline void delta_timer<time_type>::execute(std::queue<event*>& queue) {
	time_type now = m_timer->elapsed();
	time_type delta = now - m_last_run;
	m_last_run = now;
	get_ready_delta_events(delta, queue);
}

template <typename time_type>
inline void delta_timer<time_type>::insert(self_timeout_event *pe) {
	if(m_events.size() == 0){
		m_events.push_back(pe);
	} else {
		// find correct pos (do: pe->incr(-(*i).get_time() as we traverse the list)
		event_list::iterator it;
		for(it = m_events.begin();it!=m_events.end();it++) {
			if(pe->get_time() < (*it)->get_time()) 
				break;
			pe->decr((*it)->get_time());
		}
		// insert entry
		if(it != m_events.end()) {
			(*it)->decr(pe->get_time());
			m_events.insert(it, pe);
		} else {
			m_events.push_back(pe);
		}
	}
	// debug
	write_trace();
}

template <typename time_type>
inline void delta_timer<time_type>::fire_delta_events(self_time_type delta) {
	while(m_events.size()>0 && m_events.front()->get_time() <= delta) {
		delta -= m_events.front()->get_time();
		m_events.front()->fire();
		delete m_events.front();
		m_events.erase(m_events.begin());
	}
	if(m_events.size()>0) m_events.front()->incr(-delta);
}
	
template <typename time_type>
inline void delta_timer<time_type>::get_ready_delta_events(self_time_type delta, std::queue<event*>& queue) {
	while(m_events.size()>0 && m_events.front()->get_time() <= delta) {
		delta -= m_events.front()->get_time();
		queue.push(m_events.front());
		m_events.erase(m_events.begin());
	}
	if(m_events.size()>0) m_events.front()->decr(delta);
	
	// debug
	if(!queue.empty()) write_trace();
}
	
template <typename time_type>
inline void delta_timer<time_type>::write_trace() {
	std::ostringstream os;
	os << "delta_timer (";
	event_list::iterator it = m_events.begin();
	if(it != m_events.end()) {
		os << (*it)->get_time();
		it++;
	}
	for(; it != m_events.end(); it++)
		os << ", " << (*it)->get_time();
	os << ")";
	log_trace_event(os.str());
}


} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_DELTA_TIMER_H
