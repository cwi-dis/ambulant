/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/unix/unix_event_processor.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

lib::unix::event_processor::event_processor() 
  :   abstract_event_processor(lib::timer_factory(), new lib::critical_section()),
      m_is_running(false)
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
  if (m_is_running) abort();
	AM_DBG lib::logger::get_logger()->trace("event_processor 0x%x deleted", (void *)this);
}

unsigned long
lib::unix::event_processor::run()
{
        m_is_running = true;
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
	m_is_running = false;
	AM_DBG lib::logger::get_logger()->trace("event_processor 0x%x stopped", (void *)this);
	return 0;
}

void
lib::unix::event_processor::wait_event()
{
	int rv;
	struct timespec ts;
	
	// XXXX This is wrong.
	ts.tv_sec = time(NULL);
	ts.tv_nsec = 10000000; /* 10ms */
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
lib::event_processor_factory()
{
	return (event_processor *)new unix::event_processor();
}
