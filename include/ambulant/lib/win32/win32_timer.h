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

#ifndef AMBULANT_LIB_WIN32_TIMER_H
#define AMBULANT_LIB_WIN32_TIMER_H

#ifndef _INC_WINDOWS

#include <windows.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#endif // _INC_WINDOWS

#include "ambulant/config/config.h"
#include "math.h"

#ifndef AMBULANT_LIB_TIMER_H
#include "ambulant/lib/timer.h"
#endif


namespace ambulant {

namespace lib {

namespace win32 {

class win32_timer : public ambulant::lib::timer  {
  public:
	win32_timer();

	// Returns time in msec since epoch.
	// Takes into account speed with a 1% precision.
	time_type elapsed() const;

	// Gets the speed of this timer
	double get_speed() const { return 1.0;}

	// Gets the realtime speed of this
	// timer as modulated by its parent
	double get_realtime_speed() const { return 1.0;}
	signed_time_type set_drift(signed_time_type drift) { return drift; };
	signed_time_type get_drift() const { return 0; };
	void skew(signed_time_type skew);
  private:
	ULONGLONG m_epoch;

};

} // namespace win32

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_WIN32_TIMER_H
