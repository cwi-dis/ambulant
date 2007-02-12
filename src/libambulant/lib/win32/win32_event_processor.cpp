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

#include "ambulant/lib/win32/win32_event_processor.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"

#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/lib/win32/win32_timer.h"

//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

lib::win32::event_processor::event_processor(timer *t) 
:   lib::event_processor_impl(t),
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
	AM_DBG logger->debug("event_processor::run(=0x%x)", this);
#ifndef AMBULANT_PLATFORM_WIN32_WCE_3
	CoInitialize(NULL);
#endif
	while(!exit_requested()) {	
		serve_events();		
		wait_event();
	}
#ifndef AMBULANT_PLATFORM_WIN32_WCE_3
	CoUninitialize();
#endif
	AM_DBG logger->debug("event_processor::~run(=0x%x)", this);
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
