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

#include <pthread.h>

namespace ambulant {

namespace lib {

namespace unix {

class thread {
  public:
	thread()
	:	m_thread(NULL),
		m_exit_requested(false),
		m_running(false) {}

	virtual ~thread() {}

	virtual bool start() {
		if (pthread_create(&m_thread, NULL, &thread::threadproc, this) < 0 ) {
			perror("pthread_create");
		}
	}

	virtual void stop(){
		m_exit_requested = true;
		/* TODO: wake thread up */
	}
		
	bool exit_requested() {
		return m_exit_requested; 
	}
	
	bool is_running() const {
		return m_running;
	}
		
  protected:
	virtual void run() = 0;
	
	virtual void signal_exit_thread(){
		abort();
	}

  private:
	static void *threadproc(void *pParam) {
		thread* p = static_cast<thread*>(pParam);
		p->m_running = 1;
		p->run();
		p->m_running = 0;
		pthread_exit(NULL);
		return NULL;
	}

	pthread_t m_thread;
	bool m_exit_requested;
	bool m_running;
};

} // namespace unix

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_UNIX_THREAD_H
