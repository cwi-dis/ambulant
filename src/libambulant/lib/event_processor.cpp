/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

#include "ambulant/lib/delta_timer.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/logger.h"
#include <map>

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace lib;

abstract_event_processor::abstract_event_processor(abstract_timer *t) 
	:	m_timer(t),
		m_high_delta_timer(t), 
		m_med_delta_timer(t), 
		m_low_delta_timer(t)
		{ assert(t != 0); }

abstract_event_processor::~abstract_event_processor() {
		// the timer is not owned by this
}

abstract_timer *
abstract_event_processor::get_timer() const { return m_timer; }

void 
abstract_event_processor::add_event(event *pe, time_type t, 
				    event_priority priority) {

 	AM_DBG logger::get_logger()->debug("add_event(0x%x, t=%d, pri=%d)",pe,t,priority);
	m_delta_timer_cs.enter();
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
	m_delta_timer_cs.leave();
}

bool
abstract_event_processor::cancel_event(event *pe, 
				       event_priority priority) {
	bool succeeded = false;
 	AM_DBG logger::get_logger()->debug("cancel_event(0x%x, pri=%d)",pe,priority);
	m_delta_timer_cs.enter();
	switch(priority) {
		case high: 
			succeeded = m_high_delta_timer.cancel(pe);
			break;
		case med: 
			succeeded = m_med_delta_timer.cancel(pe);
			break;
		case low: 
			succeeded = m_low_delta_timer.cancel(pe);
			break;
	}
	m_delta_timer_cs.leave();
	return succeeded;
}
	
void
abstract_event_processor::cancel_all_events() {
	AM_DBG logger::get_logger()->debug("cancel_all_events()");
	m_delta_timer_cs.enter();
	m_high_delta_timer.clear();
	m_med_delta_timer.clear();
	m_low_delta_timer.clear();
 	m_delta_timer_cs.leave();
}

void 
abstract_event_processor::serve_events()
// serve all events in the high-med-low prioritity run queues
// in the right order, after checking with their delta timers
{
	// check all delta_timer queues, in the right order
	while (events_available(m_high_delta_timer, &m_high_q)
		|| events_available(m_med_delta_timer, &m_med_q)
		|| events_available(m_low_delta_timer, &m_low_q)) {
		// There was at least one event
		// First try to serve the high priority event
		if (serve_event(m_high_delta_timer, &m_high_q)) {
			// serving the event may generate another event
			// of any priority, must check all queues again
			continue;
	  	}
		// If there was no high priority event, then try to
		// serve one medium priority event
		if (serve_event(m_med_delta_timer, &m_med_q))
			// again, serving this event may generate another
			// of any priority, so check all queues
			continue;
		// There was no medium priority event either, so 
		// it must be a low priority event
		(void) serve_event(m_low_delta_timer, &m_low_q);
	}
}

bool
abstract_event_processor::events_available(delta_timer& dt, std::queue<event*> *qp)
// check, if needed, with a delta_timer to fill its run queue
// return true if the run queue contains any events
{
  	if (qp->empty()) {
		m_delta_timer_cs.enter();
		dt.execute(*qp);
		m_delta_timer_cs.leave();
	}
	return ! qp->empty();
}

bool
abstract_event_processor::serve_event(delta_timer& dt, std::queue<event*> *qp)
// serve a single event from a delta_timer run queue
// return true if an event was served
{
	bool must_serve = ! qp->empty();
	if (must_serve) {
		event *e = qp->front();
	 	AM_DBG logger::get_logger()->debug("serve_event(0x%x)",e);
		qp->pop();
		e->fire();
		delete e;
	}
	return must_serve; 
}
