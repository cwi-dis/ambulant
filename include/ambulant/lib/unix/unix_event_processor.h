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

#ifndef AMBULANT_LIB_UNIX_EVENT_PROCESSOR_H
#define AMBULANT_LIB_UNIX_EVENT_PROCESSOR_H

#include<sys/time.h>

#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/unix/unix_thread.h"
#include "ambulant/lib/mtsync.h"

#include "unix_thread.h"
#include "unix_mtsync.h"
#include "unix_timer.h"

namespace ambulant {

namespace lib {

namespace unix {

typedef ambulant::lib::unix::critical_section unix_cs;

class event_processor : 
  public ambulant::lib::event_processor_impl,
  public ambulant::lib::unix::thread {

  public:
	event_processor(timer *t);
	~event_processor();
  
  protected:
	virtual unsigned long run();
	
  private:
	void wait_event();
	void wakeup();
	pthread_cond_t m_queue_condition;
	pthread_mutex_t m_queue_mutex;	
};


} // namespace unix

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_UNIX_EVENT_PROCESSOR_H
