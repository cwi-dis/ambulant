
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#include "ambulant/lib/win32/win32_timer.h"
#include "ambulant/lib/logger.h"
#include <cmath>
#include <windows.h>

using namespace ambulant;

// Returns time in msec since epoch.
// Takes into account speed with a 1% precision.
lib::win32::win32_timer::time_type
lib::win32::win32_timer::elapsed() const {
	if(m_speed == 1.0)
		return to_millis(os_time() - m_epoch);
	// use 1% prec for speed
	ULONGLONG speed100 = int(std::floor(m_speed * 100));
	return to_millis((speed100 * (os_time() - m_epoch))/100);
}

// Sets the speed of this timer. 
void
lib::win32::win32_timer::set_speed(double speed) {
	m_epoch = os_time();
	m_speed = speed;
	speed_changed();
}

// Returns system time in msec. 
// static
lib::win32::win32_timer::time_type
lib::win32::win32_timer::os_millitime() {
	ULONGLONG msecs = win32_timer::os_time()*10000;
	return to_millis(win32_timer::os_time());
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

// Converts system units to msec.
// static 
lib::win32::win32_timer::time_type
lib::win32::win32_timer::to_millis(ULONGLONG t) {
	ULARGE_INTEGER msecs;
	msecs.QuadPart = t*10000;
	return msecs.LowPart;
}

// Factory routine for the machine-independent
// timer class
lib::abstract_timer*
lib::realtime_timer_factory() {
	return new lib::win32::win32_timer();
}
