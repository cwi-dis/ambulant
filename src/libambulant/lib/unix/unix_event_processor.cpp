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
using namespace lib;

unix::event_processor::event_processor() 
:   abstract_event_processor(timer_factory(), critical_section_factory())
{
	AM_DBG log_trace_event("event_processor 0x%x created", (void *)this);
	start();
}

unix::event_processor::~event_processor()
{
	AM_DBG log_trace_event("event_processor 0x%x deleted", (void *)this);
}

unsigned long
unix::event_processor::run()
{
	AM_DBG log_trace_event("event_processor 0x%x started", (void *)this);
	while(!exit_requested()) {	
		serve_events();		
		wait_event();
	}
	AM_DBG log_trace_event("event_processor 0x%x stopped", (void *)this);
	return 0;
}

void
unix::event_processor::wait_event()
{
	m_event_sema.down();
}

void
unix::event_processor::wakeup()
{
	AM_DBG log_trace_event("event_processor 0x%x sema-value=%d", (void *)this, m_event_sema.count());
	m_event_sema.up();
}

event_processor *
ambulant::lib::event_processor_factory()
{
	return (event_processor *)new unix::event_processor();
}