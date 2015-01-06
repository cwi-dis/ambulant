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

#ifndef AMBULANT_LIB_DELTA_TIMER_H
#define AMBULANT_LIB_DELTA_TIMER_H

#include "ambulant/config/config.h"

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

/// A scheduler for timeout events.
/// Uses delta timer pattern.
class delta_timer {
  public:
	/// A type defining how this timer represents time.
	typedef timer::time_type time_type;
	/// An event plus the delta-time it should fire.
	typedef std::pair<event*, time_type> timeout_event;

	/// Constructor.
	delta_timer(timer *t);
	virtual ~delta_timer();

	/// Fires ready events.
	/// Must be called periodically,
	/// the period defines timer resolution.
	void execute();

	/// Return list of read events.
	// Like execute but instead of executing events
	// returns the ready to fire events to the caller.
	void execute(std::queue<event*>& queue);

	/// Insert a timeout event.
	void insert(event *pe, time_type t);

	/// Cancels a scheduled event.
	// Returns true on success.
	bool cancel(event *pe);

	/// Clear all events.
	void clear();

	/// Return the delay until the next event, or a large number.
	time_type next_event_time() const;

	/// debug output.
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
