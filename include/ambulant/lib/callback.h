
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

/* 
 * Callback mechanism implementation.
 *
 * See lib/win32/win32_test_processor.h
 * for a usage example.
 *
 * The current implementation assumes and requires
 * the first template argument (the callback target)
 * to be ref counted.
 */
 
#ifndef AMBULANT_LIB_CALLBACK_H
#define AMBULANT_LIB_CALLBACK_H

#include <memory>

#ifndef AMBULANT_LIB_EVENT_H
#include "lib/event.h"
#endif

#ifndef AMBULANT_LIB_REFCOUNT_H
#include "lib/refcount.h"
#endif

namespace ambulant {

namespace lib {

// T is the target object class that should be ref_counted
// A is the callback argument class.
// This callback becomes the owner of the argument 
// (e.g. it is responsible to delete arg) 

template <class T, class A, typename time_type = unsigned long>
class callback : public timeout_event<time_type> {

  public:
	typedef time_type self_time_type;

	// Callback member function signature
	typedef void (T::*MF)(A *a);
  
  private:
	// The target object that should be ref_counted.
	T *m_obj;
	
	// The taget member function.
	MF m_mf;
	
	// The argument to be passed to the member function.
	// This object is the owner of the argument object.
	A *m_arg;
  	
  	// relative time remaining
  	self_time_type m_timeout;
  	
  public:
	// 'obj' is the target object having a member function 'mf' accepting 'arg' 
	callback(T* obj, MF mf, A* arg, self_time_type timeout = 0)
	: m_obj(obj), m_mf(mf), m_arg(arg), m_timeout(timeout) {
		if(obj != 0) obj->add_ref();
	}
	
	~callback() {
		delete m_arg;
		release_target();
	}

	// event interface implementation
	virtual void fire() {
		if(m_mf != 0 && m_obj != 0)
			(m_obj->*m_mf)(m_arg);
		release_target();
	}
	
	// timeout_event interface implementation
	virtual self_time_type get_time() { return m_timeout;}
	virtual void incr(self_time_type dt) { m_timeout += dt;}
	virtual void decr(self_time_type dt) 
		{ m_timeout = (dt>m_timeout)?0:(m_timeout - dt);}
	virtual void set_timeout(self_time_type t) { m_timeout = t;}
	
  private:
	void release_target() {
		if(m_obj != 0) {
			ref_counted* obj = dynamic_cast<ref_counted*>(m_obj);
			obj->release();
			m_obj = 0;
		}
	}
	
};

 
} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_CALLBACK_H
