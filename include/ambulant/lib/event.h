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

#ifndef AMBULANT_LIB_EVENT_H
#define AMBULANT_LIB_EVENT_H

#include "ambulant/config/config.h"

namespace ambulant {

namespace lib {

/// Interface to be provided by a scheduler/timer event.
class event {
  public:
	virtual ~event() {}

	/// Called to fire the event.
	virtual void fire() = 0;
};

enum event_priority {ep_low, ep_med, ep_high};


/// Convenience event class that sets a flag when the event fires.
class flag_event : public event {
  public:
	/// Pass a reference to the boolean that should be set.
	flag_event(bool& flag)
	:	m_flag(flag) {}

	void fire() {
		m_flag = !m_flag;
	}
  public:
	bool& m_flag;	///< The flag that is set when the event fires.
};

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_EVENT_H
