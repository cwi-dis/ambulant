/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_LIB_UNIX_THREAD_H
#define AMBULANT_LIB_UNIX_THREAD_H

#include "ambulant/lib/thread.h"
#include <pthread.h>
#undef unix

namespace ambulant {

namespace lib {

namespace unix {

class thread : public ambulant::lib::thread {
  public:
	thread();
	virtual ~thread();

	virtual bool start();
	virtual void stop();
	bool terminate();
		
	bool exit_requested() const;
	bool is_running() const;
		
  protected:
	virtual unsigned long run() = 0;
	
	virtual void signal_exit_thread();

  private:
	static void *threadproc(void *pParam);
	
	pthread_t m_thread;
	bool m_exit_requested;
	bool m_exit_done;
	bool m_running;
};

} // namespace unix

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_UNIX_THREAD_H
