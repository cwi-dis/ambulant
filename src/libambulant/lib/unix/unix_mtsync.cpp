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

#include "ambulant/lib/unix/unix_mtsync.h"
#include "ambulant/lib/logger.h"
#include <stdlib.h>

using namespace ambulant;
#undef unix

lib::unix::critical_section::critical_section()
{
	if (pthread_mutex_init(&m_cs, NULL) < 0) {
		lib::logger::get_logger()->fatal("unix_critical_section: pthread_mutex_init failed: %s", strerror(errno));
	}
}

lib::unix::critical_section::~critical_section()
{
	if (pthread_mutex_destroy(&m_cs) < 0) {
		lib::logger::get_logger()->fatal("unix_critical_section: pthread_mutex_destroy failed: %s", strerror(errno));
	}
}

void
lib::unix::critical_section::enter()
{
	if (pthread_mutex_lock(&m_cs) < 0) {
		lib::logger::get_logger()->fatal("unix_critical_section: pthread_mutex_lock failed: %s", strerror(errno));
	}
}

void
lib::unix::critical_section::leave()
{
	if (pthread_mutex_unlock(&m_cs) < 0) {
		lib::logger::get_logger()->fatal("unix_critical_section: pthread_mutex_unlock failed: %s", strerror(errno));
	}
}

lib::unix::condition::condition()
{
	if (pthread_cond_init(&m_condition, NULL) < 0) {
		lib::logger::get_logger()->fatal("lib::unix::condition(): pthread_cond_init failed: %s", strerror(errno));
	}
}

lib::unix::condition::~condition()
{
}

void
lib::unix::condition::signal()
{
	if (pthread_cond_signal(&m_condition) < 0) {
		lib::logger::get_logger()->fatal("lib::unix::condition::signal(): pthread_cond_signal failed: %s", strerror(errno));
	}
}

void
lib::unix::condition::signal_all()
{
	if (pthread_cond_broadcast(&m_condition) < 0) {
		lib::logger::get_logger()->fatal("lib::unix::condition::signal_all(): pthread_cond_broadcast failed: %s", strerror(errno));
	}
}

bool
lib::unix::condition::wait(int microseconds, critical_section &cs)
{
	int rv;
	
	if (microseconds >= 0) {
		struct timespec ts;
		struct timeval tv;
		int dummy;
		dummy = gettimeofday(&tv,NULL);
		ts.tv_sec = tv.tv_sec;
		ts.tv_nsec = (tv.tv_usec + microseconds)* 1000;
		if (ts.tv_nsec > 1000000000) {
			ts.tv_sec += 1;
			ts.tv_nsec -= 1000000000;
		}
		rv = pthread_cond_timedwait(&m_condition, &cs.m_cs, &ts);
	} else {
		rv = pthread_cond_timedwait(&m_condition, &cs.m_cs, NULL);
	}
	if (rv < 0) {
		if (errno != ETIMEDOUT)
			lib::logger::get_logger()->fatal("lib::unix::condition::wait(): pthread_cond_wait failed: %s", strerror(errno));
		return false;
	}
	return true;
}

#if 0
lib::unix::counting_semaphore::counting_semaphore()
:	m_lock(critical_section()),
	m_wait(critical_section()),
	m_count(0)
{
	// The semaphore is initialized empty, so lock the wait mutex
	m_wait.enter();
}

lib::unix::counting_semaphore::~counting_semaphore()
{
}

void
lib::unix::counting_semaphore::down()
{
	m_lock.enter();
	m_count--;
	if (m_count < 0) {
		m_lock.leave();
		m_wait.enter();
	} else {
		m_lock.leave();
	}
}

void
lib::unix::counting_semaphore::up()
{
	m_lock.enter();
	m_count++;
	if (m_count <= 0) {
		m_wait.leave();
	} 
	m_lock.leave();
}

int
lib::unix::counting_semaphore::count()
{
	m_lock.enter();
	int rv = m_count;
	m_lock.leave();
	return rv;
}
#endif
