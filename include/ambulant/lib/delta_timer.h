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
	typedef abstract_timer::time_type time_type;
	typedef std::pair<event*, time_type> timeout_event;
	
	delta_timer(abstract_timer *t);
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
	abstract_timer *m_timer;
	};

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_DELTA_TIMER_H
