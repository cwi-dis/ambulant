
/* 
 * @$Id$ 
 */

#include "ambulant/lib/unix/unix_timer.h"
#include "ambulant/lib/logger.h"
#include <sys/time.h>

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
lib::abstract_timer *
lib::realtime_timer_factory()
{
	return new lib::unix::unix_timer();
}
