/* 
 * @$Id$ 
 */

#ifndef AMBULANT_LIB_WIN32_EVENT_PROCESSOR_H
#define AMBULANT_LIB_WIN32_EVENT_PROCESSOR_H

#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/timer.h"

#include "win32_thread.h"

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

namespace ambulant {

namespace lib {

namespace win32 {

class event_processor : 
  public ambulant::lib::abstract_event_processor,
  public ambulant::lib::win32::thread {
  
  public:
	event_processor(abstract_timer *t);	
	~event_processor();
    
  protected:
	virtual unsigned long run(); 
	
  private:
	// wait until some thread calls wakeup
	void wait_event();
	
	// wakes up this thread
	void wakeup(); 

	HANDLE m_wait_event;
	enum { RESOLUTION = 100};
};


} // namespace win32

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_WIN32_EVENT_PROCESSOR_H

