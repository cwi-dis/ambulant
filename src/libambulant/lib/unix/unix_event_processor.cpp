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
:   abstract_event_processor(lib::timer_factory(), new lib::critical_section())
{
	AM_DBG lib::logger::get_logger()->trace("event_processor 0x%x created", (void *)this);
	start();
}

lib::unix::event_processor::~event_processor()
{
	AM_DBG lib::logger::get_logger()->trace("event_processor 0x%x deleted", (void *)this);
}

unsigned long
lib::unix::event_processor::run()
{
	AM_DBG lib::logger::get_logger()->trace("event_processor 0x%x started", (void *)this);
	while(!exit_requested()) {	
		serve_events();		
		wait_event();
	}
	AM_DBG lib::logger::get_logger()->trace("event_processor 0x%x stopped", (void *)this);
	return 0;
}

void
lib::unix::event_processor::wait_event()
{
	m_event_sema.down();
}

void
lib::unix::event_processor::wakeup()
{
	m_event_sema.up();
}

lib::event_processor *
lib::event_processor_factory()
{
	return (event_processor *)new unix::event_processor();
}