
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
 */
 
#ifndef AMBULANT_LIB_CALLBACK_H
#define AMBULANT_LIB_CALLBACK_H

// a callback implements event
#include "ambulant/lib/event.h"

// callback targets may be ref counted
#include "ambulant/lib/refcount.h"

namespace ambulant {

namespace lib {

////////////////////////////
// callback_struct

// A structure able to hold the
// generic callback arguments.
// T is the target object class
// A is the callback argument class.
template <class T, class A>
struct callback_struct {
	// Callback member function signature
	typedef void (T::*MF)(A *a);
	
	// The target object that should be ref_counted.
	T *m_obj;
	
	// The taget member function.
	MF m_mf;
	
	// The argument to be passed to the member function.
	// This object is the owner of the argument object.
	A *m_arg;
	
	// struct constructor
	callback_struct(T* obj, MF mf, A* arg) 
	:	m_obj(obj), m_mf(mf), m_arg(arg) {}
};


////////////////////////////
// callback_event

// Not ref counted version of a callback.
//
// This callback becomes the owner of the argument 
// (e.g. it is responsible to delete arg)
// 
// The target object is not ref counted and 
// should exist when this fires.

template <class T, class A>
class callback_event : public event, 
	private callback_struct<T, A> {
  public:
	// 'obj' is the target object having a member function 'mf' accepting 'arg' 
	callback_event(T* obj, MF mf, A* arg);
	
	// deletes arg	
	~callback_event();

	// event interface implementation
	virtual void fire();
};


////////////////////////////
// callback

// Ref counted version of a callback.
//
// This callback becomes the owner of the argument 
// (e.g. it is responsible to delete arg) 
//
// The target object is ref counted.

template <class T, class A>
class callback : public event, 
	private callback_struct<T, A> {
  public:
	// 'obj' is the target object having a member function 'mf' accepting 'arg' 
	callback(T* obj, MF mf, A* arg);
	
	// deletes arg, releases target tref
	~callback();
	
	// event interface implementation
	virtual void fire();
};


/////////////////////////
// Inline callback_event implementation

// 'obj' is the target object having a member 
// function 'mf' accepting 'arg' 
template <class T, class A>
inline callback_event<T, A>::callback_event(T* obj, MF mf, A* arg)
:	callback_struct<T, A>(obj, mf, arg) {
}
	
// deletes arg, releases target ref	
template <class T, class A>
inline callback_event<T, A>::~callback_event() {
	delete m_arg;
}

// event interface implementation
template <class T, class A>
inline void callback_event<T, A>::fire() {
	if(m_mf != 0 && m_obj != 0)
		(m_obj->*m_mf)(m_arg);
}

//////////////////////
// Inline callback implementation

template <class T, class A>
inline callback<T, A>::callback(T* obj, MF mf, A* arg)
:	callback_struct<T, A>(obj, mf, arg) {
	if(obj) obj->add_ref();
}

template <class T, class A>
inline callback<T, A>::~callback() {
	delete m_arg;
	if(m_obj != 0) m_obj->release();
}

// event interface implementation
template <class T, class A>
inline void callback<T, A>::fire() {
	if(m_mf != 0 && m_obj != 0)
		(m_obj->*m_mf)(m_arg);
}	

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_CALLBACK_H
