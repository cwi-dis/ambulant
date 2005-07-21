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
  private:		
	ULONGLONG m_epoch;
	
};

} // namespace win32
 
} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_WIN32_TIMER_H
