
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_WIN32_TIMER_H
#define AMBULANT_LIB_WIN32_TIMER_H

#ifndef AMBULANT_LIB_TIMER_H
#include "lib/timer.h"
#endif

namespace ambulant {

namespace lib {

namespace win32 {

// simple win32 os timer
class os_timer : public ambulant::lib::timer<unsigned long>  {
  public:
	os_timer() : m_start_time(GetTickCount()) {}
	virtual self_time_type elapsed() const { return GetTickCount()-m_start_time;}
	virtual void restart() { m_start_time = GetTickCount();}
	
  private:
	self_time_type m_start_time;
};


} // namespace win32
 
} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_WIN32_TIMER_H
