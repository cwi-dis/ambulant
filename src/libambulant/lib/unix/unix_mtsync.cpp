
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/unix/unix_mtsync.h"
#include <stdlib.h>

using namespace ambulant;
using namespace lib;
#undef unix

unix::critical_section::critical_section()
{
	if (pthread_mutex_init(&m_cs, NULL) < 0) abort();
}

unix::critical_section::~critical_section()
{
	if (pthread_mutex_destroy(&m_cs) < 0) abort();
}

void
unix::critical_section::enter()
{
	if (pthread_mutex_lock(&m_cs) < 0) abort();
}

void
unix::critical_section::leave()
{
	if (pthread_mutex_unlock(&m_cs) < 0) abort();
}

critical_section *
ambulant::lib::critical_section_factory()
{
	return (critical_section *)new unix::critical_section();
}

unix::counting_semaphore::counting_semaphore()
:	m_lock(critical_section()),
	m_wait(critical_section()),
	m_count(0)
{
	m_wait.leave();
}

unix::counting_semaphore::~counting_semaphore()
{
}

void
unix::counting_semaphore::down()
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

void unix::counting_semaphore::up()
{
	m_lock.enter();
	m_count++;
	if (m_count <= 0) {
		m_wait.leave();
	}
	m_lock.leave();
}
