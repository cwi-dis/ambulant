
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

#ifndef AMBULANT_LIB_TIMER_H
#include "lib/timer.h"
#endif

#include <time.h>

namespace ambulant {

namespace lib {

namespace unix {

// XXX: time() returns secs. This timer should be msec based. 

// simple unix os timer
class os_timer : public ambulant::lib::timer  {
  public:
	os_timer() : m_start_time(time(NULL)) {}
	virtual time_type elapsed() const { return time(NULL)-m_start_time;}
	virtual void restart() { m_start_time = time(NULL);}
	
  private:
	time_type m_start_time;
};


} // namespace unix
 
} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_UNIX_TIMER_H
