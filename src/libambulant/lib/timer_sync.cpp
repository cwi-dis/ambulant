// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
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

#include "ambulant/lib/timer_sync.h"
#include "ambulant/lib/logger.h"

#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#ifdef WITH_REMOTE_SYNC
using namespace ambulant;


class timer_sync_impl : public timer_sync {
  public:
	timer_sync_impl()
	:	m_timer(NULL)
	{
	}
	
	virtual ~timer_sync_impl() {
        AM_DBG lib::logger::get_logger()->debug("timer_sync(0x%x): deleted", (void*)this);
		if (m_timer) {
			m_timer->set_observer(NULL);
		}
	}
	
  
	void initialize(timer_control *timer) {
		m_timer = timer;
		AM_DBG lib::logger::get_logger()->debug("timer_sync(0x%x): initialize(0x%x)", (void*)this, (void*)m_timer);
        m_timer->set_observer(this);
	}
	
	void started() {
		AM_DBG lib::logger::get_logger()->debug("timer_sync(0x%x): timer 0x%x: started", (void*)this, (void*)m_timer);
	}
	
	void stopped() {
		AM_DBG lib::logger::get_logger()->debug("timer_sync(0x%x): timer 0x%x: stopped", (void*)this, (void*)m_timer);
	}
	
	void paused() {
		AM_DBG lib::logger::get_logger()->debug("timer_sync(0x%x): timer 0x%x: paused", (void*)this, (void*)m_timer);
	}
	
	void resumed() {
		AM_DBG lib::logger::get_logger()->debug("timer_sync(0x%x): timer 0x%x: resumed", (void*)this, (void*)m_timer);
	}
	
  private:
	timer_control* m_timer;
};

timer_sync *
timer_sync_factory_impl::new_timer_sync(document *doc)
{
	timer_sync *rv = new timer_sync_impl();
	AM_DBG lib::logger::get_logger()->debug("timer_sync_factory(0x%x) -> 0x%x", doc, rv);
	return rv;
}
#endif