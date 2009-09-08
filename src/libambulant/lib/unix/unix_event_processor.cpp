// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2008 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

/* 
 * @$Id$ 
 */
#if 0
#include "ambulant/lib/unix/unix_event_processor.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

lib::unix::event_processor::event_processor(timer *t) 
:   event_processor_impl(t)
{
	AM_DBG lib::logger::get_logger()->debug("event_processor 0x%x created", (void *)this);
    m_lock.enter();
	start();
    m_lock.leave();
}

lib::unix::event_processor::~event_processor()
{
	stop();
    m_lock.enter();
	assert( ! is_running());
	AM_DBG lib::logger::get_logger()->debug("event_processor 0x%x deleted", (void *)this);
    m_lock.leave();
}

unsigned long
lib::unix::event_processor::run()
{
	AM_DBG lib::logger::get_logger()->debug("event_processor 0x%x started", (void *)this);
	m_lock.enter();
	while(!exit_requested()) {	
		_serve_events();		
		_wait_event();
	}
    m_lock.leave();
	AM_DBG lib::logger::get_logger()->debug("event_processor 0x%x stopped", (void *)this);
	return 0;
}

bool
lib::unix::event_processor::_wait_event()
{
	// Note: m_lock must be locked at this call
    return m_lock.wait(10000);
}

void
lib::unix::event_processor::_wakeup()
{
    m_lock.signal();
}

lib::event_processor *
lib::event_processor_factory(timer *t)
{
	return new unix::event_processor(t);
}
#endif