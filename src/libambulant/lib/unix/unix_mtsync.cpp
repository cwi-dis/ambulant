
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/unix/unix_mtsync.h"
#include "ambulant/lib/logger.h"
#include <stdlib.h>

using namespace ambulant;
#undef unix

lib::unix::critical_section::critical_section()
{
	if (pthread_mutex_init(&m_cs, NULL) < 0) abort();
}

lib::unix::critical_section::~critical_section()
{
	if (pthread_mutex_destroy(&m_cs) < 0) abort();
}

void
lib::unix::critical_section::enter()
{
	if (pthread_mutex_lock(&m_cs) < 0) abort();
}

void
lib::unix::critical_section::leave()
{
	if (pthread_mutex_unlock(&m_cs) < 0) abort();
}

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
