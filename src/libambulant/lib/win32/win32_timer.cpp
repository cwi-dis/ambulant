
/* 
 * @$Id$ 
 */

#include "ambulant/lib/win32/win32_timer.h"
#include "ambulant/lib/logger.h"
#include <cmath>
#include <windows.h>

using namespace ambulant;

const ULONGLONG MILLIS_FACT = 10000;

lib::win32::win32_timer::win32_timer() 
:	m_epoch(os_time()), 
	m_speed(1.0) {
}

// Returns time in msec since epoch.
// Takes into account speed with a 1% precision.
lib::win32::win32_timer::time_type
lib::win32::win32_timer::elapsed() const {
	ULONGLONG dt = os_time() - m_epoch;
	if(m_speed == 1.0)
		return time_type(dt/MILLIS_FACT);
	ULONGLONG speed100 = ULONGLONG(std::floor(0.5 + m_speed * 100));
	ULONGLONG edt = (speed100 * dt ) / 100;
	return time_type(edt/MILLIS_FACT);
}

// Sets the speed of this timer. 
void
lib::win32::win32_timer::set_speed(double speed) {
	m_epoch = os_time();
	m_speed = speed;
	speed_changed();
}

// Returns system time in system units (0.1 micro-sec units or 0.0001 msec). 
// static
ULONGLONG 
lib::win32::win32_timer::os_time() {
	FILETIME ft;
	SYSTEMTIME st;
	GetSystemTime(&st);              
	SystemTimeToFileTime(&st, &ft);
	ULARGE_INTEGER li = {ft.dwLowDateTime, ft.dwHighDateTime};
	return li.QuadPart;
}

// Factory routine for the machine-independent
// timer class
lib::abstract_timer*
lib::realtime_timer_factory() {
	return new lib::win32::win32_timer();
}
