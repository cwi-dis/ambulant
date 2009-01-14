/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2008 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_LIB_EVENT_PROCESSOR_H
#define AMBULANT_LIB_EVENT_PROCESSOR_H

#include "ambulant/config/config.h"

#include "ambulant/lib/event.h"
#include "ambulant/lib/timer.h"

namespace ambulant {

namespace lib {

#ifndef WITHOUT_DELAYED_REDRAW
class AMBULANTAPI event_processor_observer {
  public:
	  virtual ~event_processor_observer() {};
	virtual void lock_redraw() = 0;
	virtual void unlock_redraw() = 0;
};
#endif

/// Interface to be provided by an event scheduler.
class event_processor {
  public:
	typedef timer::time_type time_type;
	
	virtual ~event_processor() {}
	
	/// schedule an event pe to fire at relative time t (millis) in the provided priority class.
	virtual void add_event(event *pe, time_type t, event_priority priority) = 0;

	/// Cancel all events.
	virtual void cancel_all_events() = 0;

	/// Cancel a previously scheduled event.
	virtual bool cancel_event(event *pe, event_priority priority = ep_low) = 0;
	
	// Fires waiting events.
	virtual void serve_events() = 0;

	// Get the underlying timer.
	virtual timer *get_timer() const = 0;
	
	// Stop this event processor (stops the underlying thread).
	virtual void stop_processor_thread() = 0;

#ifndef WITHOUT_DELAYED_REDRAW
	// Set the observer.
	virtual void set_observer(event_processor_observer *obs) = 0;
#endif
};

} // namespace lib

} // namespace ambulant



/////////////////////////////////////////////////
// event_processor_impl

// event_processor_impl is not the owner of timer.


#include <queue>

#ifndef AMBULANT_PLATFORM_WIN32_WCE_3
#include <cassert>
#endif

#include "ambulant/lib/logger.h"
#include "ambulant/lib/delta_timer.h"
#include "ambulant/lib/mtsync.h"

namespace ambulant {

namespace lib {

/// Implementation of event_processor.
/// This is the machine-independent portion of the event_processor.
/// There is a machine-dependent companion class that glues
/// this together with a (machine-dependent) thread to get the
/// complete behaviour.
class event_processor_impl : public event_processor {
  public:
 	event_processor_impl(timer *t);	
	~event_processor_impl();	
	timer *get_timer() const;
	
	void add_event(event *pe, time_type t, event_priority priority);
	bool cancel_event(event *pe, event_priority priority = ep_low);
	void cancel_all_events();
	void serve_events();
	void stop_processor_thread() {};
#ifndef WITHOUT_DELAYED_REDRAW
	void set_observer(event_processor_observer *obs) {m_observer = obs; };
#endif
#ifndef NDEBUG
	void dump();
#endif
  protected:
	// called by add_event
	// wakes up thread executing serve_events
	virtual void wakeup() = 0;
	
	// wait until some thread calls wakeup
	virtual void wait_event() = 0;

	// the timer for this processor
	timer *m_timer;
#ifndef WITHOUT_DELAYED_REDRAW
	event_processor_observer *m_observer;
#endif
 private:
	// check, if needed, with a delta_timer to fill its run queue
	// return true if the run queue contains any events
	bool events_available(delta_timer& dt, std::queue<event*> *qp);

	// serve a single event from a delta_timer run queue
	// return true if an event was served
	bool serve_event(delta_timer& dt, std::queue<event*> *qp);
	
	// high priority delta timer and its event queue
	delta_timer m_high_delta_timer;
	std::queue<event*> m_high_q;

	// medium priority delta timer and its event queue
	delta_timer m_med_delta_timer;
	std::queue<event*> m_med_q;
	
	// low priority delta timer and its event queue
	delta_timer m_low_delta_timer;
	std::queue<event*> m_low_q;
	
	// protects delta timer lists
	critical_section m_delta_timer_cs;  
};

/// Machine-dependent factory function
AMBULANTAPI event_processor *event_processor_factory(timer *t);

} // namespace lib

} // namespace ambulant


#endif // AMBULANT_LIB_EVENT_PROCESSOR_H
