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

#include "ambulant/lib/unix/unix_timer.h"
#include "ambulant/lib/logger.h"
#include <sys/time.h>
#include <assert.h>

using namespace ambulant;

lib::unix::unix_timer::time_type
lib::unix::unix_timer::elapsed() const
{
	return os_millitime();
}

void
lib::unix::unix_timer::set_speed(double speed)
{
	lib::logger::get_logger()->fatal("unix_timer: cannot set speed of realtime timer");
}

void
lib::unix::unix_timer::skew(signed_time_type sk)
{
	assert(sk == 0);
}

lib::unix::unix_timer::time_type
lib::unix::unix_timer::os_millitime()
{
	struct timeval tv;
	static time_t epoch = 0;

	if (gettimeofday(&tv, NULL) < 0) return 0;
	if (epoch == 0)
		epoch = tv.tv_sec;
	return (tv.tv_sec-epoch)*1000 + tv.tv_usec / 1000;
}

// Factory routine for the machine-independent
// timer class
lib::timer *
lib::realtime_timer_factory()
{
	return new lib::unix::unix_timer();
}
