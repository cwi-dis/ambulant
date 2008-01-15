/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2008 Stichting CWI, 
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

#ifndef AMBULANT_LIB_UNIX_MTSYNC_H
#define AMBULANT_LIB_UNIX_MTSYNC_H

#include <errno.h>

#include <pthread.h>

#include "ambulant/lib/abstract_mtsync.h"

namespace ambulant {

namespace lib {
#undef unix
namespace unix {

class critical_section : public ambulant::lib::base_critical_section {
	friend class condition;
  public:
	critical_section();
	~critical_section();

	void enter();
	void leave();

  private:
	pthread_mutex_t m_cs;
};

class condition : public ambulant::lib::base_condition {
  public:
	condition();
	~condition();
	
	void signal();
	void signal_all();
	bool wait(int microseconds, critical_section &cs);
  private:
	pthread_cond_t m_condition;
};

class counting_semaphore {
  public:
	counting_semaphore();
	~counting_semaphore();
	
	void down();
	void up();
	int count();
  private:
    critical_section m_lock;
    critical_section m_wait;
    int m_count;
};

} // namespace unix

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_UNIX_MTSYNC_H
