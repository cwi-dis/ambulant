// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2010 Stichting CWI,
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/*
 * @$Id$
 */

#include "ambulant/lib/delta_timer.h"
#include "ambulant/lib/lib_gcd_event_processor_impl.h"
#include "ambulant/lib/logger.h"
#include <map>

#ifdef WITH_LIBXDISPATCH
#include "xdispatch/xdispatch/dispatch.h"
#else
#include <dispatch/dispatch.h>
#endif

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace lib;

lib::event_processor *
lib::gcd_event_processor_factory(timer *t)
{
	return new event_processor_impl_gcd(t);
}

event_processor_impl_gcd::event_processor_impl_gcd(timer *t)
:	m_timer(t),
	m_observer(NULL)
{
	m_lock.enter();
	assert(t != 0);
	m_lock.leave();
}

event_processor_impl_gcd::~event_processor_impl_gcd()
{
	AM_DBG lib::logger::get_logger()->debug("event_processor 0x%x deleted", (void *)this);
	cancel_all_events();
}

timer *
event_processor_impl_gcd::get_timer() const
{
	return m_timer;
}

void gb_serve_event(event *gb_e)
{
#ifdef AMBULANT_PLATFORM_WIN32
	logger::get_logger()->debug("serve_event: ThreadId=0x%x pe=0x%x", GetCurrentThreadId(), gb_e);
#endif
	AM_DBG logger::get_logger()->debug("before serve_event(0x%x)in GCD_WIN",gb_e);
	gb_e->fire();
	AM_DBG logger::get_logger()->debug("after serve_event(0x%x)in GCD_WIN",gb_e);
	delete gb_e;
}

void
event_processor_impl_gcd::add_event(event *pe, time_type t,
								event_priority priority)
{
	
	AM_DBG logger::get_logger()->debug("lib_gcd_event_processor_impl:add_event(0x%x, t=%d, pri=%d)",pe,t,priority);
	m_lock.enter();
	// Insert the event into the correct queue.
	int prio;
#ifdef WITH_LIBXDISPATCH
	switch (priority) {
	case ep_high:
		prio = xdispatch::HIGH;
		break;
	case ep_med:
		prio = xdispatch::DEFAULT;
		break;
	case ep_low:
		prio = xdispatch::LOW;
		break;
	default:
		assert(0);
	}
	xdispatch::global_queue(prio).after(
		${
			pe->fire();
			delete pe;
			logger::get_logger()->debug("serve_event(0x%x)in GCD_WIN",pe);
		},
		dispatch_time(DISPATCH_TIME_NOW, t*1000000));

#else
	switch (priority) {
	case ep_high:
		prio = DISPATCH_QUEUE_PRIORITY_HIGH;
		break;
	case ep_med:
		prio = DISPATCH_QUEUE_PRIORITY_DEFAULT;
		break;
	case ep_low:
		prio = DISPATCH_QUEUE_PRIORITY_LOW;
		break;
	default:
		assert(0);
	}
	AM_DBG logger::get_logger()->debug("t=%ld, pe=0x%x",t, pe);
#ifdef AMBULANT_PLATFORM_WIN32
	logger::get_logger()->debug("add_event(%d): ThreadId=0x%x pe=0x%x", priority, GetCurrentThreadId(),pe);
#endif
	dispatch_after_f(
		dispatch_time(DISPATCH_TIME_NOW, t*1000000),
		dispatch_get_global_queue(prio, 0),
		pe,
		(dispatch_function_t)gb_serve_event);

#endif // WITH_LIBXDISPATCH

	m_lock.leave();
}

bool
event_processor_impl_gcd::cancel_event(event *pe, event_priority priority)
{
	bool succeeded = false;
	AM_DBG logger::get_logger()->debug("cancel_event(0x%x, pri=%d)",pe,priority);
	return succeeded;
}

void
event_processor_impl_gcd::cancel_all_events()
{
	AM_DBG logger::get_logger()->debug("cancel_all_events()");
	// Your application does not need to retain or release the global (main and concurrent) dispatch queues; 
	// calling this function on global dispatch queues has no effect.
}

