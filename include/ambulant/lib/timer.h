
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
 

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_TIMER_H
