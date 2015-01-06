// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include <windows.h>

#include "ambulant/lib/win32/win32_timer.h"
#include "ambulant/lib/logger.h"

#include <cmath>
#include <cassert>

using namespace ambulant;

const ULONGLONG MILLIS_FACT = 10000;
// Returns system time in system units (0.1 micro-sec units or 0.0001 msec).
static DWORD
os_time() {
	FILETIME ft;
	SYSTEMTIME st;
	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ft);
	ULARGE_INTEGER li = {ft.dwLowDateTime, ft.dwHighDateTime};
	return (DWORD)(li.QuadPart/MILLIS_FACT);
}

lib::win32::win32_timer::win32_timer()
:	m_epoch(os_time()) {
}

// Returns time in msec since epoch.
// Takes into account speed with a 1% precision.
lib::win32::win32_timer::time_type
lib::win32::win32_timer::elapsed() const {
	assert((DWORD)m_epoch == m_epoch);
	DWORD dt = os_time() - (DWORD)m_epoch;
	return time_type(dt);
}

void
lib::win32::win32_timer::skew(signed_time_type skew)
{
	assert(skew==0);
}

// Factory routine for the machine-independent
// timer class
lib::timer*
lib::realtime_timer_factory() {
	return new lib::win32::win32_timer();
}


