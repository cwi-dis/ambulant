// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
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

#include "ambulant/lib/unix/unix_event_processor.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

lib::unix::event_processor::event_processor(timer *t) 
:   event_processor_impl(t)
{
  	pthread_mutex_init(&m_queue_mutex, NULL);
	pthread_cond_init(&m_queue_condition, NULL);
	AM_DBG lib::logger::get_logger()->debug("event_processor 0x%x created", (void *)this);
	if (pthread_mutex_init(&m_queue_mutex, NULL) < 0) {
		lib::logger::get_logger()->fatal("unix_event_processor: pthread_mutex_init failed: %s", strerror(errno));
	}
	if (pthread_cond_init(&m_queue_condition, NULL) < 0) {
		lib::logger::get_logger()->fatal("unix_event_processor: pthread_cond_init failed: %s", strerror(errno));
	}
	start();
}

lib::unix::event_processor::~event_processor()
{
	stop();
	AM_DBG lib::logger::get_logger()->debug("event_processor 0x%x deleted", (void *)this);
}

unsigned long
lib::unix::event_processor::run()
{
	AM_DBG lib::logger::get_logger()->debug("event_processor 0x%x started", (void *)this);
	// XXXX Note: the use of the mutex means that only one thread is actively
	// serving events. This needs to be rectified at some point: only the
	// queue manipulations should be locked with the mutex.
	if (pthread_mutex_lock(&m_queue_mutex) < 0 ) {
		lib::logger::get_logger()->fatal("unix_event_processor.run: pthread_mutex_lock failed: %s", strerror(errno));
	}
	while(!exit_requested()) {	
		serve_events();		
		wait_event();
	}
	pthread_mutex_unlock(&m_queue_mutex);
	AM_DBG lib::logger::get_logger()->debug("event_processor 0x%x stopped", (void *)this);
	return 0;
}

void
lib::unix::event_processor::wait_event()
{
	int rv;
	struct timespec ts;
	struct timeval tv;
	int dummy;
		
	
	// we want to wait for 10ms
	dummy = gettimeofday(&tv,NULL);
	ts.tv_sec = tv.tv_sec;
	ts.tv_nsec = (tv.tv_usec + 10000)* 1000;
	if (ts.tv_nsec > 1000000000) {
		ts.tv_sec += 1;
		ts.tv_nsec -= 1000000000;
	}
	
	rv = pthread_cond_timedwait(&m_queue_condition, &m_queue_mutex, &ts);
	if ( rv < 0 && errno != ETIMEDOUT) {
		lib::logger::get_logger()->fatal("unix_event_processor.wait_event: pthread_cond_wait failed: %s", strerror(errno));
	}
}

void
lib::unix::event_processor::wakeup()
{
	if (pthread_cond_signal(&m_queue_condition) < 0) {
		lib::logger::get_logger()->fatal("unix_event_processor.wakeup: pthread_cond_signal failed: %s", strerror(errno));
	}
}

lib::event_processor *
lib::event_processor_factory(timer *t)
{
	return new unix::event_processor(t);
}
