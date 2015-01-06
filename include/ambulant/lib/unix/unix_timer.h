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

#ifndef AMBULANT_LIB_UNIX_TIMER_H
#define AMBULANT_LIB_UNIX_TIMER_H

#include "ambulant/lib/timer.h"
#undef unix
namespace ambulant {

namespace lib {

namespace unix {

// XXX: time() returns secs. This timer should be msec based.

// simple unix os timer
class unix_timer : public ambulant::lib::timer  {
  public:
	unix_timer() {};

	time_type elapsed() const;
	void set_speed(double speed);
	double get_realtime_speed() const { return 1.0; }
	signed_time_type set_drift(signed_time_type drift) { return drift; };
	signed_time_type get_drift() const { return 0; };
	void skew(signed_time_type skew);

  private:
	static time_type os_millitime();
};


} // namespace unix

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_UNIX_TIMER_H
