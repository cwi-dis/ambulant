
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#ifndef AMBULANT_LIB_UNIX_MTSYNC_H
#define AMBULANT_LIB_UNIX_MTSYNC_H

#include <pthread.h>

#include "ambulant/lib/mtsync.h"

namespace ambulant {

namespace lib {
#undef unix
namespace unix {

class critical_section : public ambulant::lib::critical_section {
  public:
	critical_section() { if (pthread_mutex_init(&m_cs, NULL) < 0) abort();}
	~critical_section() { if (pthread_mutex_destroy(&m_cs) < 0) abort();}

	void enter() { if (pthread_mutex_lock(&m_cs) < 0) abort();}
	void leave() { if (pthread_mutex_unlock(&m_cs) < 0) abort();}

  private:
	pthread_mutex_t m_cs;
};

class counting_semaphore {
  public:
	counting_semaphore()
	:	m_lock(critical_section()),
		m_wait(critical_section()),
		m_count(0) {
		m_wait.leave();
	}
	
	~counting_semaphore() {
	}
	
	void down() {
		m_lock.enter();
		m_count--;
		if (m_count < 0) {
			m_lock.leave();
			m_wait.enter();
		} else {
			m_lock.leave();
		}
	}
	
	void up() {
		m_lock.enter();
		m_count++;
		if (m_count <= 0) {
			m_wait.leave();
		}
		m_lock.leave();
	}
  private:
    critical_section m_lock;
    critical_section m_wait;
    int m_count;
};

} // namespace unix

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_UNIX_MTSYNC_H
