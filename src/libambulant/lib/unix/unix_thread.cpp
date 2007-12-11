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

#include "ambulant/lib/unix/unix_thread.h"
#include "ambulant/lib/logger.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#undef terminate

using namespace ambulant;

lib::unix::thread::thread()
:	m_exit_requested(false),
    m_exit_done(false),
	m_running(false),
	m_started(false)
{
}

lib::unix::thread::~thread()
{
	if (m_running || (m_started && !m_exit_done))
		stop();
}

bool 
lib::unix::thread::start()
{
	if (m_started) return false;
	
    assert(!m_running);
    assert(!m_exit_requested);
    assert(!m_exit_done);
	m_started = true;
	if (pthread_create(&m_thread, NULL, &thread::threadproc, this) < 0 ) {
		perror("pthread_create");
		abort();
		//m_starting = false;
	}
	return true;
}

void
lib::unix::thread::stop()
{
    assert(!m_exit_done);
	m_exit_requested = true;
	/* TODO: wake thread up */
	if (pthread_equal(m_thread, pthread_self())) {
		lib::logger::get_logger()->debug("thread::stop(0x%x) called by self", m_thread);
	} else {
		if (pthread_join(m_thread, NULL) < 0) {
			perror("pthread_join in unix::thread::stop()");
		}
	}
	m_exit_done = true;
}
	
bool
lib::unix::thread::terminate()
{
	// No idea what to do here...
	abort();
	return false;
}
	
bool
lib::unix::thread::exit_requested() const
{
	return m_exit_requested; 
}

bool
lib::unix::thread::is_running() const
{
	return m_running;
}
	
void
lib::unix::thread::signal_exit_thread(){
	// Don't know how to do this, yet.
	abort();
}

void *
lib::unix::thread::threadproc(void *pParam)
{
	thread* p = static_cast<thread*>(pParam);
	assert(p->m_started);
	assert(!p->m_running);
	assert(!p->m_exit_done);
	p->m_running = true;
	if (!p->m_exit_requested) (void)p->run();
	p->m_running = false;
	pthread_exit(NULL);
	return NULL;
}
