
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

// The default argument (unsigned long) is intented 
// to be used for measuring the time elapsed in msecs.
// Could be used also for secs but its general usefulness 
// is questionable since integral secs is 
// too large a unit for the mm/gui domain. 
// For secs one could use timer<double>.
// For finer granularity one could use timer<unsigned __int64>.
// e.g. timer<_ULONGLONG>.

template <typename time_type = unsigned long>
class timer {
  public:
	virtual ~timer() {}
	
	// this timer time type
	typedef time_type self_time_type;
	
	// returns the time elapsed
	// e.g. return (time_now>ref_time)?time_now - ref_time:0;
	virtual self_time_type elapsed() const = 0;
	
	// resets the reference time to now
	virtual void restart() = 0;
	
	//virtual void set_speed(double speed) = 0;
};
 

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_TIMER_H
