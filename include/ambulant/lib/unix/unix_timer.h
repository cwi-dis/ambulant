
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_UNIX_TIMER_H
#define AMBULANT_LIB_UNIX_TIMER_H

#include "ambulant/lib/timer.h"
#undef unix
namespace ambulant {

namespace lib {

namespace unix {

// XXX: time() returns secs. This timer should be msec based. 

// simple unix os timer
class os_timer : public ambulant::lib::timer  {
  public:
	os_timer();
	time_type elapsed() const;
	void restart();
	
	static time_type millitime() ;
  private:
	time_type m_start_time;
};


} // namespace unix
 
} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_UNIX_TIMER_H
