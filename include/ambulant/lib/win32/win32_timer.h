
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_WIN32_TIMER_H
#define AMBULANT_LIB_WIN32_TIMER_H

#ifndef AMBULANT_LIB_TIMER_H
#include "ambulant/lib/timer.h"
#endif

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

namespace ambulant {

namespace lib {

namespace win32 {

class win32_timer : public ambulant::lib::abstract_timer  {
  public:
	win32_timer() : m_epoch(0), m_speed(1.0) {}
	
	// Returns time in msec since epoch.
	// Takes into account speed with a 1% precision.	
	time_type elapsed() const;
	
	// Sets the speed of this timer. 	
	void set_speed(double speed);
	
	// Gets the speed of this timer.
	// XXX: Should be called get_local_speed/get_effective_speed	
	double get_realtime_speed() const { return m_speed;}
	
  private:
  
	// Returns system time in system units (0.1 micro-sec units or 0.0001 msec).
	static ULONGLONG os_time();
	
	// Returns system time in msec. 	
	static time_type os_millitime();
	
	// Converts system units to msec.
	static time_type to_millis(ULONGLONG t);
	
	ULONGLONG m_epoch;
	double m_speed;
	
};

} // namespace win32
 
} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_WIN32_TIMER_H
