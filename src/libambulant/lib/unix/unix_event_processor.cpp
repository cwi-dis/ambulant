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

#include "ambulant/lib/unix/unix_event_processor.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

lib::unix::event_processor::event_processor(timer *t) 
:   abstract_event_processor(t)
{
  	pthread_mutex_init(&m_queue_mutex, NULL);
	pthread_cond_init(&m_queue_condition, NULL);
	AM_DBG lib::logger::get_logger()->trace("event_processor 0x%x created", (void *)this);
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
	AM_DBG lib::logger::get_logger()->trace("event_processor 0x%x deleted", (void *)this);
}

unsigned long
lib::unix::event_processor::run()
{
	AM_DBG lib::logger::get_logger()->trace("event_processor 0x%x started", (void *)this);
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
	AM_DBG lib::logger::get_logger()->trace("event_processor 0x%x stopped", (void *)this);
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
	if (ts.tv_nsec > 1000000) {
		ts.tv_sec += 1;
		ts.tv_nsec -= 1000000;
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
