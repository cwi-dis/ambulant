// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/lib/delta_timer.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/mtsync.h"
#ifdef AMBULANT_PLATFORM_UNIX
#include "ambulant/lib/unix/unix_thread.h"
#define BASE_THREAD lib::unix::thread
#endif
#ifdef AMBULANT_PLATFORM_WIN32
#include "ambulant/lib/win32/win32_thread.h"
#define BASE_THREAD lib::win32::thread
#endif

#ifdef WITH_GCD_EVENT_PROCESSOR
#ifdef WITH_LIBXDISPATCH
#include "xdispatch/xdispatch/dispatch.h"
#else
#include "dispatch/dispatch.h"
#endif // WITH_LIBXDISPATCH
#endif // WITH_GCD_EVENT_PROCESSOR

#include <map>
#include <queue>
#include <cassert>

#if defined(AMBULANT_PLATFORM_WIN32)
#include <windows.h>
#endif

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

//#if 0

using namespace ambulant;
using namespace lib;

/// Implementation of event_processor.
/// This is an implementation of the event_processor interface that uses a thread
/// and three priority queues to execute events.
class event_processor_impl : public event_processor, public BASE_THREAD {
  public:
	/// Constructor.
	event_processor_impl(timer *t);
	~event_processor_impl();

	timer *get_timer() const;
	/// Internal: code run by the thread.
	unsigned long run();

	void add_event(event *pe, time_type t, event_priority priority);
	bool cancel_event(event *pe, event_priority priority = ep_low);
	void cancel_all_events();
	void set_observer(event_processor_observer *obs) {m_observer = obs; };
	/// Debug method to dump queues.
	void dump();
  protected:
	/// Called by platform-specific subclasses.
	/// Should hold m_lock when calling.
	void _serve_events();

	timer *m_timer;	///< the timer for this processor
	event_processor_observer *m_observer;	///< The observer.

	/// protects whole data structure.
	critical_section_cv m_lock;

  private:
	// check, if needed, with a delta_timer to fill its run queue
	// return true if the run queue contains any events
	bool _events_available(delta_timer& dt, std::queue<event*> *qp);

	// serve a single event from a delta_timer run queue
	// return true if an event was served
	bool _serve_event(delta_timer& dt, std::queue<event*> *qp, event_priority priority);

	// high priority delta timer and its event queue
	delta_timer m_high_delta_timer;
	std::queue<event*> m_high_q;

	// medium priority delta timer and its event queue
	delta_timer m_med_delta_timer;
	std::queue<event*> m_med_q;

	// low priority delta timer and its event queue
	delta_timer m_low_delta_timer;
	std::queue<event*> m_low_q;
	
#ifdef WITH_GCD_EVENT_PROCESSOR
	dispatch_group_t m_gcd_group;
#endif

};

lib::event_processor *
lib::event_processor_factory(timer *t)
{
	return new event_processor_impl(t);
}

event_processor_impl::event_processor_impl(timer *t)
:	m_timer(t),
	m_high_delta_timer(t),
	m_med_delta_timer(t),
	m_low_delta_timer(t),
	m_observer(NULL)
{
	m_lock.enter();
	assert(t != 0);
#ifdef WITH_GCD_EVENT_PROCESSOR
	m_gcd_group = dispatch_group_create();
#endif
	start();
	m_lock.leave();
}

event_processor_impl::~event_processor_impl()
{
	AM_DBG lib::logger::get_logger()->debug("event_processor 0x%x deleted", (void *)this);
	stop();
	assert( ! is_running());
	cancel_all_events();
#ifdef WITH_GCD_EVENT_PROCESSOR
	dispatch_group_wait(m_gcd_group, DISPATCH_TIME_FOREVER);
	dispatch_release(m_gcd_group);
#endif // WITH_GCD_EVENT_PROCESSOR
}

timer *
event_processor_impl::get_timer() const
{
	return m_timer;
}

unsigned long
event_processor_impl::run()
{
	AM_DBG lib::logger::get_logger()->debug("event_processor 0x%x started", (void *)this);
#if defined(AMBULANT_PLATFORM_WIN32)
	HRESULT hr;
#if defined(COINIT_MULTITHREADED)
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
	hr = CoInitialize(NULL);
#endif // COINIT_MULTITHREADED
	if (hr) {
		lib::logger::get_logger()->trace("win32_event_processor::run: CoInitializeEx failed with 0x%x", hr);
	}
#endif // AMBULANT_PLATFORM_WIN32
	m_lock.enter();
	while(!exit_requested()) {
		_serve_events();
		(void)m_lock.wait(10000);
	}
	m_lock.leave();
#if defined(AMBULANT_PLATFORM_WIN32)
	CoUninitialize();
#endif
	AM_DBG lib::logger::get_logger()->debug("event_processor 0x%x stopped", (void *)this);
	return 0;
}

void
event_processor_impl::add_event(event *pe, time_type t,
					event_priority priority)
{

	AM_DBG logger::get_logger()->debug("add_event(0x%x, t=%d, pri=%d)",pe,t,priority);
	m_lock.enter();
	// Insert the event into the correct queue.
	switch(priority) {
		case ep_high:
			m_high_delta_timer.insert(pe, t);
			break;
		case ep_med:
			m_med_delta_timer.insert(pe, t);
			break;
		case ep_low:
			m_low_delta_timer.insert(pe, t);
			break;
	}
	// Signal the event handler thread
	m_lock.signal();
	m_lock.leave();
}

bool
event_processor_impl::cancel_event(event *pe, event_priority priority)
{
#ifdef	WITH_GCD_EVENT_PROCESSOR
	assert(0); // not implementable for GCD
#endif//WITH_GCD_EVENT_PROCESSOR
	bool succeeded = false;
	AM_DBG logger::get_logger()->debug("cancel_event(0x%x, pri=%d)",pe,priority);
	m_lock.enter();
	switch(priority) {
		case ep_high:
			succeeded = m_high_delta_timer.cancel(pe);
			break;
		case ep_med:
			succeeded = m_med_delta_timer.cancel(pe);
			break;
		case ep_low:
			succeeded = m_low_delta_timer.cancel(pe);
			break;
	}
	m_lock.leave();
	return succeeded;
}

void
event_processor_impl::cancel_all_events()
{
	AM_DBG logger::get_logger()->debug("cancel_all_events()");
	m_lock.enter();
	m_high_delta_timer.clear();
	m_med_delta_timer.clear();
	m_low_delta_timer.clear();
	m_lock.leave();
}

// serve all events in the high-med-low prioritity run queues
// in the right order, after checking with their delta timers
void
event_processor_impl::_serve_events()
{
	if (m_observer) {
		event_processor_observer *obs = m_observer;
		m_lock.leave();
		obs->lock_redraw();
		m_lock.enter();
		if (m_observer == NULL) {
			// We got deleted behind our back
			lib::logger::get_logger()->debug("event_processor: cannot unlock_redraw() because observer disappeared");
			m_lock.leave();
			return;
		}
	}
	// check all delta_timer queues, in the right order
	while (_events_available(m_high_delta_timer, &m_high_q)
		|| _events_available(m_med_delta_timer, &m_med_q)
		|| _events_available(m_low_delta_timer, &m_low_q))
	{
		AM_DBG lib::logger::get_logger()->debug("_serve_events: %d hi, %d med, %d lo", m_high_q.size(), m_med_q.size(), m_low_q.size());
		// There was at least one event
		// First try to serve the high priority event
		if (_serve_event(m_high_delta_timer, &m_high_q, ep_high)) {
			// serving the event may generate another event
			// of any priority, must check all queues again
			continue;
		}
		// If there was no high priority event, then try to
		// serve one medium priority event
		if (_serve_event(m_med_delta_timer, &m_med_q, ep_med))
			// again, serving this event may generate another
			// of any priority, so check all queues
			continue;
		// There was no medium priority event either, so
		// it must be a low priority event
		(void) _serve_event(m_low_delta_timer, &m_low_q, ep_low);
	}

	timer::signed_time_type drift = m_timer->get_drift();
	if (drift > 0) {
		// If the clock is behind, we set it forward. But we don't advance it past the
		// next event that is due to be scheduled.
		// If the clock is too fast we simply set it back.
		timer::signed_time_type next_event_time = std::min(
			m_low_delta_timer.next_event_time(),
			std::min(
				m_med_delta_timer.next_event_time(),
				m_high_delta_timer.next_event_time()));
		if (drift >= next_event_time)
			drift = next_event_time-1;
	}
	AM_DBG if (drift) lib::logger::get_logger()->debug("event_processor: adjust clock %d ms (positive is forward)", drift);
	m_timer->skew(drift);

	if (m_observer) {
		m_lock.leave();
		m_observer->unlock_redraw();
		m_lock.enter();
	}
}

bool
event_processor_impl::_events_available(delta_timer& dt, std::queue<event*> *qp)
// check, if needed, with a delta_timer to fill its run queue
// return true if the run queue contains any events
{
	if (qp->empty()) {
//		m_lock.leave();
		dt.execute(*qp);
//		m_lock.enter();
	}
	return !qp->empty();
}

#ifdef WITH_GCD_EVENT_PROCESSOR
// Helper function, called whtn the event should fire.
static void
gb_serve_event_1(event *gb_e)
{
	AM_DBG logger::get_logger()->debug("serve_event(0x%x)in GCD",gb_e);
	gb_e->fire();
	delete gb_e;
}
#endif // WITH_GCD_EVENT_PROCESSOR

bool
event_processor_impl::_serve_event(delta_timer& dt, std::queue<event*> *qp, event_priority priority)
// serve a single event from a delta_timer run queue
// return true if an event was served
{
	bool must_serve = !qp->empty();
	if (must_serve) {
		event *e = qp->front();
		AM_DBG logger::get_logger()->debug("serve_event(0x%x)",e);
		qp->pop();
		m_lock.leave();
#ifdef WITH_GCD_EVENT_PROCESSOR
#ifdef WITH_LIBXDISPATCH
		xdispatch::global_queue(xdispatch::DEFAULT).async(${
			e->fire();
			delete e;
			logger::get_logger()->debug("serve_event(0x%x)in GCD_WIN",e);
		});
#else
		int prio;
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
		dispatch_group_async_f(m_gcd_group, dispatch_get_global_queue(prio, 0), e, (dispatch_function_t)gb_serve_event_1);
#endif // WITH_LIBXDISPATCH
		 
#else
		e->fire();
		delete e;
#endif // WITH_GCD_EVENT_PROCESSOR

		m_lock.enter();
	}
	return must_serve;
}

void
event_processor_impl::dump()
{
#ifndef NDEBUG
//	std::queue<event*>::iterator i;
	lib::logger::get_logger()->trace("event_processor_impl[0x%x]::dump():", (void *)this);
	lib::logger::get_logger()->trace("high waiting:");
	m_high_delta_timer.write_trace();
//	lib::logger::get_logger()->trace("high runnable:");
//	for (i=m_high_q.begin(); i != m_high_q.end(); i++)
//		lib::logger::get_logger()->trace("	0x%x", (void *)*i);
	lib::logger::get_logger()->trace("med waiting:");
	m_med_delta_timer.write_trace();
//	lib::logger::get_logger()->trace("med runnable:");
//	for (i=m_med_q.begin(); i != m_med_q.end(); i++)
//		lib::logger::get_logger()->trace("	0x%x", (void *)*i);
	lib::logger::get_logger()->trace("low waiting:");
	m_low_delta_timer.write_trace();
//	lib::logger::get_logger()->trace("low runnable:");
//	for (i=m_low_q.begin(); i != m_low_q.end(); i++)
//		lib::logger::get_logger()->trace("	0x%x", (void *)*i);
#endif
}
//#endif
