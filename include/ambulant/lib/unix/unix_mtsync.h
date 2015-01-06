/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

  protected:
	pthread_mutex_t m_cs;
};

class critical_section_cv : public critical_section, public ambulant::lib::base_critical_section_cv {
  public:
	critical_section_cv();
	~critical_section_cv();

	void signal();
	bool wait(int microseconds = -1);
  private:
	pthread_cond_t m_condition;
};

} // namespace unix

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_UNIX_MTSYNC_H
