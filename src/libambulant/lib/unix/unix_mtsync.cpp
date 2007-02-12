// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
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

#include "ambulant/lib/unix/unix_mtsync.h"
#include "ambulant/lib/logger.h"
#include <stdlib.h>
#include <sys/time.h>

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
		rv = pthread_cond_wait(&m_condition, &cs.m_cs);
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
