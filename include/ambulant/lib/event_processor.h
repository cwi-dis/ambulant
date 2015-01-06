/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

#ifndef AMBULANT_LIB_EVENT_PROCESSOR_H
#define AMBULANT_LIB_EVENT_PROCESSOR_H

#include "ambulant/config/config.h"

#include "ambulant/lib/event.h"
#include "ambulant/lib/timer.h"

namespace ambulant {

namespace lib {

/// Notification interface for event processor activity.
/// Before and after the event_processor starts processing a batch of events
/// it notifies implementors of this interface. This can be used to delay redraws
/// (so we don't get a cascade of redraws where a single one at the end would
/// be sufficient).
class AMBULANTAPI event_processor_observer {
  public:
	virtual ~event_processor_observer() {};
	/// Called before the event_processor starts on a batch of events.
	virtual void lock_redraw() = 0;
	/// Called after the event_processor finishes a batch of events.
	virtual void unlock_redraw() = 0;
};

/// Interface to be provided by an event scheduler.
class event_processor {
  public:
	/// How this event_processor represents time.
	typedef timer::time_type time_type;

	virtual ~event_processor() {}

	/// schedule an event pe to fire at relative time t (millis) in the provided priority class.
	virtual void add_event(event *pe, time_type t, event_priority priority) = 0;

	/// Cancel all events.
	virtual void cancel_all_events() = 0;

	/// Cancel a previously scheduled event.
	virtual bool cancel_event(event *pe, event_priority priority = ep_low) = 0;

	/// Get the underlying timer.
	virtual timer *get_timer() const = 0;

	/// Signal interest in getting event_processor_observer callbacks.
	virtual void set_observer(event_processor_observer *obs) = 0;
};

/// Factory function
AMBULANTAPI event_processor *event_processor_factory(timer *t);

} // namespace lib

} // namespace ambulant


#endif // AMBULANT_LIB_EVENT_PROCESSOR_H
