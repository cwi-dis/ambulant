
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
	critical_section();
	~critical_section();

	void enter();
	void leave();

  private:
	pthread_mutex_t m_cs;
};

class counting_semaphore {
  public:
	counting_semaphore();
	~counting_semaphore();
	
	void down();
	void up();
	int count();
  private:
    critical_section m_lock;
    critical_section m_wait;
    int m_count;
};

} // namespace unix

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_UNIX_MTSYNC_H
