/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/unix/unix_event_processor.h"

using namespace ambulant;
using namespace lib;

unix::event_processor::event_processor() 
:   abstract_event_processor(timer_factory(), critical_section_factory())
{
}

unix::event_processor::~event_processor()
{
}

unsigned long
unix::event_processor::run()
{
	log_trace_event("event_processor started");
	while(!exit_requested()) {	
		serve_events();		
		wait_event();
	}
	log_trace_event("event_processor stopped");
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
	m_event_sema.up();
}

event_processor *
ambulant::lib::event_processor_factory()
{
	return (event_processor *)new unix::event_processor();
}