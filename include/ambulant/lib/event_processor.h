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
	virtual void cancel_all_events() = 0;
	
	// serves waiting events 
	virtual void serve_events() = 0;

	// Get the underlying timer
	virtual timer *get_timer() const = 0;
};

} // namespace lib

} // namespace ambulant



/////////////////////////////////////////////////
// abstract_event_processor

// abstract_event_processor is not the owner of abstract_timer.


#include <queue>
#include <cassert>

#include "ambulant/lib/logger.h"
#include "ambulant/lib/delta_timer.h"
#include "ambulant/lib/mtsync.h"

namespace ambulant {

namespace lib {

class abstract_event_processor : public event_processor {
  public:
	abstract_event_processor(timer *t) 
	:	m_timer(t),
		m_high_delta_timer(t), 
		m_med_delta_timer(t), 
		m_low_delta_timer(t)
		{ assert(t); }
	
	~abstract_event_processor() {
		// the timer is not owned by this
	}
	
	timer *get_timer() const { return m_timer; }
	
	void add_event(event *pe, time_type t, event_priority priority = low) {
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
	
	void cancel_all_events() {
		m_delta_timer_cs.enter();
		m_high_delta_timer.clear();
		m_med_delta_timer.clear();
		m_low_delta_timer.clear();
 		m_delta_timer_cs.leave();
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
		m_delta_timer_cs.enter();
		dt.execute(queue);
 		m_delta_timer_cs.leave();
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
	critical_section m_delta_timer_cs;  
};

// Machine-dependent factory function
event_processor *event_processor_factory(timer *t);

} // namespace lib

} // namespace ambulant


#endif // AMBULANT_LIB_EVENT_PROCESSOR_H
