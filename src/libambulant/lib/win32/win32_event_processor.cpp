
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/win32/win32_event_processor.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"

#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/lib/win32/win32_timer.h"

using namespace ambulant;

lib::win32::event_processor::event_processor() 
:   lib::abstract_event_processor(new os_timer(), new lib::critical_section()),
	m_wait_event(0) {
	m_wait_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(m_wait_event == 0)
		win_report_last_error("CreateEvent()");	
	start();
}
	
lib::win32::event_processor::~event_processor() {
	if(m_wait_event) 
		CloseHandle(m_wait_event);
}
    
unsigned long 
lib::win32::event_processor::run() {
	lib::logger* logger = lib::logger::get_logger();
	logger->trace("event_processor started");
	while(!exit_requested()) {	
		serve_events();		
		wait_event();
	}
	logger->trace("event_processor stopped");
	return 0;
}
	
// wait until some thread calls wakeup
void lib::win32::event_processor::wait_event() {
	ResetEvent(m_wait_event);
	WaitForSingleObject(m_wait_event, RESOLUTION);
}
	
// wakes up this thread
void lib::win32::event_processor::wakeup() {
	SetEvent(m_wait_event);
}
