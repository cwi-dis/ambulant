
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_TIMER_H
#define AMBULANT_LIB_TIMER_H

namespace ambulant {

namespace lib {

class timer {
  public:
	// this timer time type (assumed in msecs)
	typedef unsigned long time_type;
	
	virtual ~timer() {}
		
	// returns the time elapsed
	// e.g. return (time_now>ref_time)?time_now - ref_time:0;
	virtual time_type elapsed() const = 0;
	
	// resets the reference time to now
	virtual void restart() = 0;
	
	//virtual void set_speed(double speed) = 0;
};

// A (machine-dependent) routine to create a timer object
timer *timer_factory();

} // namespace lib
 
} // namespace ambulant


////////////////////////////
// crt_timer 
// a simple timer based on the c-runtime-library function clock() 
// may fail for too long intervals
// ( ~ 36 minutes for CLOCKS_PER_SEC = 1000000)

#include <time.h>
#include <math.h>

namespace ambulant {

namespace lib {

class crt_timer : public timer {
  public:
	crt_timer() : m_start_time(clock()) {}
	
	virtual time_type elapsed() const { return conv(clock() - m_start_time);}
	virtual void restart() { m_start_time = clock();}
	
  private:
	clock_t m_start_time;
	
	static time_type conv(clock_t ctv) { 
		return (time_type)floor(0.5 + 1000.0 * (ctv / double(CLOCKS_PER_SEC)));
	}
};

} // namespace lib
 
} // namespace ambulant


#endif // AMBULANT_LIB_TIMER_H


