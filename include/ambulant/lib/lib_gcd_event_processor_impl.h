/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2010 Stichting CWI,
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

/*
 * @$Id$
 */

#ifndef AMBULANT_LIB_GCD_EVENT_PROCESSOR_H
#define AMBULANT_LIB_GCD_EVENT_PROCESSOR_H

#include "ambulant/lib/event_processor.h"

/////////////////////////////////////////////////
// event_processor_impl_gcd

// event_processor_impl_gcd is not the owner of timer.


#include <cassert>

#include "ambulant/lib/logger.h"
#include "ambulant/lib/delta_timer.h"
#include "ambulant/lib/mtsync.h"

#undef EVENT_PROCESSOR_WITH_LOCK

namespace ambulant {

namespace lib {

class event_processor_impl_gcd;

#ifdef EVENT_PROCESSOR_WITH_LOCK
typedef struct call_back_param {
	event_processor_impl_gcd *m_pointer_event_processor;
	event *m_pointer_event;
} gb_call_back_param;
#endif

/// Implementation of event_processor by using GCD.
class event_processor_impl_gcd : public event_processor {
public:
	event_processor_impl_gcd(timer *t);
	~event_processor_impl_gcd();
	
	timer *get_timer() const; 
	//unsigned long run();
	
	void add_event(event *pe, time_type t, event_priority priority);
#ifdef EVENT_PROCESSOR_WITH_LOCK
	static	void gb_serve_event(gb_call_back_param *p_struct);
#endif
	bool cancel_event(event *pe, event_priority priority = ep_low);
	void cancel_all_events();
	void set_observer(event_processor_observer *obs) {m_observer = obs; };

protected:
	// Called by platform-specific subclasses.
	// Should hold m_lock when calling.
	//void _serve_events();
	
	// the timer for this processor
	timer *m_timer;
	event_processor_observer *m_observer;
	
	// protects whole data structure
	critical_section_cv m_lock;
#ifdef EVENT_PROCESSOR_WITH_LOCK
	critical_section m_lock_cb;
#endif
};

	
/// factory function
AMBULANTAPI event_processor *gcd_event_processor_factory(timer *t);

} // namespace lib

} // namespace ambulant


#endif // AMBULANT_LIB_GCD_EVENT_PROCESSOR_H
