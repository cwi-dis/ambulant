
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

// callback is a kind of event
#include "ambulant/lib/event.h"

// callback target is ref counted
#include "ambulant/lib/refcount.h"

namespace ambulant {

namespace lib {

// T is the target object class that should be ref_counted
// A is the callback argument class.
// This callback becomes the owner of the argument 
// (e.g. it is responsible to delete arg) 

template <class T, class A>
class callback : public event {

  public:
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
  	
  public:
	// 'obj' is the target object having a member function 'mf' accepting 'arg' 
	callback(T* obj, MF mf, A* arg);
	
	// deletes arg, releases target ref	
	~callback();

	// event interface implementation
	virtual void fire();
	
  private:
	
	// releases target ref
	void release_target();
	
};


/////////////////////////
// Inline callback implementation

// 'obj' is the target object having a member 
// function 'mf' accepting 'arg' 
template <class T, class A>
inline callback<T, A>::callback(T* obj, MF mf, A* arg)
:	m_obj(obj), m_mf(mf), m_arg(arg) {
		if(obj != 0) obj->add_ref();
}
	
// deletes arg, releases target ref	
template <class T, class A>
inline callback<T, A>::~callback() {
	delete m_arg;
	release_target();
}

// event interface implementation
template <class T, class A>
inline void callback<T, A>::fire() {
	if(m_mf != 0 && m_obj != 0)
		(m_obj->*m_mf)(m_arg);
	release_target();
}	

// releases target ref
template <class T, class A>
inline void callback<T, A>::release_target() {
	if(m_obj != 0) {
		ref_counted* obj = dynamic_cast<ref_counted*>(m_obj);
		obj->release();
		m_obj = 0;
	}
}

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_CALLBACK_H
