/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_LIB_WIN32_EVENT_PROCESSOR_H
#define AMBULANT_LIB_WIN32_EVENT_PROCESSOR_H

#ifndef _INC_WINDOWS

#include <windows.h>
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#endif

#include "ambulant/config/config.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/timer.h"

#include "win32_thread.h"


namespace ambulant {

namespace lib {

namespace win32 {

class event_processor : 
  public ambulant::lib::event_processor_impl,
  public ambulant::lib::win32::thread {
  
  public:
	event_processor(timer *t);	
	~event_processor();
    
    virtual void stop_processor_thread() {
		thread::stop();
    }
    
  protected:
	virtual unsigned long run(); 
	
  private:
	// wait until some thread calls wakeup
	void wait_event();
	
	// wakes up this thread
	void wakeup(); 

	HANDLE m_wait_event;
	enum { RESOLUTION = 50};
};


} // namespace win32

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_WIN32_EVENT_PROCESSOR_H

