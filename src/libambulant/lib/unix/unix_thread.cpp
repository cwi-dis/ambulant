/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/unix/unix_thread.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#undef terminate

using namespace ambulant;

lib::unix::thread::thread()
:	m_thread(NULL),
	m_exit_requested(false),
	m_running(false)
{
}

lib::unix::thread::~thread()
{
}

bool 
lib::unix::thread::start()
{
	if (pthread_create(&m_thread, NULL, &thread::threadproc, this) < 0 ) {
		perror("pthread_create");
	}
	return false;
}

void
lib::unix::thread::stop()
{
	m_exit_requested = true;
	/* TODO: wake thread up */
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
	p->m_running = 1;
	(void)p->run();
	p->m_running = 0;
	pthread_exit(NULL);
	return NULL;
}
