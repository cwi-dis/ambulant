/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

#include "ambulant/lib/win32/win32_event_processor.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/mtsync.h"

#include "ambulant/lib/win32/win32_error.h"
#include "ambulant/lib/win32/win32_timer.h"

using namespace ambulant;

lib::win32::event_processor::event_processor(timer *t) 
:   lib::abstract_event_processor(t),
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
	logger->trace("event_processor::run(=0x%x)", this);
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
	logger->trace("event_processor::~run(=0x%x)", this);
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
