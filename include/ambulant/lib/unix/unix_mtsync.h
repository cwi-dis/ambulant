
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

namespace ambulant {

namespace lib {

namespace unix {

class critical_section {
  public:
	critical_section() { if (pthread_mutex_init(&m_cs, NULL) < 0) abort();}
	~critical_section() { if (pthread_mutex_destroy(&m_cs) < 0) abort();}

	void enter() { if (pthread_mutex_lock(&m_cs) < 0) abort();}
	void leave() { if (pthread_mutex_unlock(&m_cs) < 0) abort();}

  private:
	pthread_mutex_t m_cs;
};

} // namespace unix

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_UNIX_MTSYNC_H
