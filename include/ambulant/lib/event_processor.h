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

#include "ambulant/config/config.h"

#include "ambulant/lib/event.h"
#include "ambulant/lib/timer.h"

namespace ambulant {

namespace lib {

/// Interface to be provided by an event scheduler.
class event_processor {
  public:
	typedef timer::time_type time_type;
	enum event_priority {low, med, high};
	
	virtual ~event_processor() {}
	
	/// schedule an event pe to fire at relative time t (millis) in the provided priority class.
	virtual void add_event(event *pe, time_type t, event_priority priority) = 0;

	/// Cancel all events.
	virtual void cancel_all_events() = 0;

	/// Cancel a previously scheduled event.
	virtual bool cancel_event(event *pe, event_priority priority = low) = 0;
	
	// Fires waiting events.
	virtual void serve_events() = 0;

	// Get the underlying timer.
	virtual timer *get_timer() const = 0;
	
	// Stop this event processor (stops the underlying thread).
	virtual void stop_processor_thread() = 0;
};

} // namespace lib

} // namespace ambulant



/////////////////////////////////////////////////
// abstract_event_processor

// abstract_event_processor is not the owner of abstract_timer.


#include <queue>

#ifndef AMBULANT_PLATFORM_WIN32_WCE_3
#include <cassert>
#endif

#include "ambulant/lib/logger.h"
#include "ambulant/lib/delta_timer.h"
#include "ambulant/lib/mtsync.h"

namespace ambulant {

namespace lib {

/// Implementation of event_processor (with a badly chosen name).
/// This is actually a concrete implementation of the
/// event_processor interface.
class abstract_event_processor : public event_processor {
  public:
 	abstract_event_processor(timer *t);	
	~abstract_event_processor();	
	timer *get_timer() const;
	
	void add_event(event *pe, time_type t, event_priority priority);
	bool cancel_event(event *pe, event_priority priority = low);
	void cancel_all_events();
	void serve_events();
	void stop_processor_thread() {};
	
  protected:
	// called by add_event
	// wakes up thread executing serve_events
	virtual void wakeup() = 0;
	
	// wait until some thread calls wakeup
	virtual void wait_event() = 0;

	// the timer for this processor
	timer *m_timer;

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

// Machine-dependent factory function
AMBULANTAPI event_processor *event_processor_factory(timer *t);

} // namespace lib

} // namespace ambulant


#endif // AMBULANT_LIB_EVENT_PROCESSOR_H
