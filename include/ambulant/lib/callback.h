
/* 
 * @$Id$ 
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
	// The target object.
	T *m_obj;
	
	// The taget member function.
	void (T::*m_mf)(A *a);
	
	// The argument to be passed to the member function.
	// This object is the owner of the argument object.
	A *m_arg;
	
	// struct constructor
	callback_struct(T* obj, void (T::*mf)(A *a), A* arg) 
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
	callback_event(T* obj, void (T::*mf)(A *a), A* arg);
	
	// deletes arg	
	~callback_event();

	// event interface implementation
	virtual void fire();
};


// Apropriate for built-in types arguments (int, double, etc)
// May be used also for simple structures like point and size.
// May be used for built-in types arguments
// or arguments that are references or const references to objects
template <class T, class A>
class scalar_arg_callback_event : public event {
	T *m_obj;
	void (T::*m_mf)(A a);
	A m_arg;
	
  public:
	scalar_arg_callback_event(T* obj, void (T::*mf)(A a), A arg) 
	:	m_obj(obj), m_mf(mf), m_arg(arg) {}
	
	virtual void fire() { 
		if(m_obj && m_mf)
			(m_obj->*m_mf)(m_arg);
	}
};


template <class T>
class no_arg_callback_event : public event {
  public:
	T *m_obj;
	void (T::*m_mf)();
  
	no_arg_callback_event(T* obj, void (T::*mf)())
	:	m_obj(obj), m_mf(mf) {}
		
	virtual void fire() {
		if(m_obj && m_mf)
			(m_obj->*m_mf)();
	}
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
	callback(T* obj, void (T::*mf)(A *a), A* arg);
	
	// deletes arg, releases target tref
	~callback();
	
	// event interface implementation
	virtual void fire();
};

// Apropriate for built-in types arguments (int, double, etc)
// May be used also for simple structures like point and size.
// May be used for built-in types arguments
// or arguments that are references or const references to objects
template <class T, class A>
class scalar_arg_callback : public event {
	T *m_obj;
	void (T::*m_mf)(A a);
	A m_arg;
	
  public:
	scalar_arg_callback(T* obj, void (T::*mf)(A a), A arg) 
	:	m_obj(obj), m_mf(mf), m_arg(arg) {
		if(m_obj) m_obj->add_ref();
	}
	~scalar_arg_callback() {
		if(m_obj) m_obj->release();
	}	
	
	virtual void fire() { 
		if(m_obj && m_mf)
			(m_obj->*m_mf)(m_arg);
	}
};


template <class T>
class no_arg_callback : public event {
  public:
	T *m_obj;
	
	void (T::*m_mf)();
  
	no_arg_callback(T* obj, void (T::*mf)())
	:	m_obj(obj), m_mf(mf) {
		if(m_obj) m_obj->add_ref();
	}
	
	~no_arg_callback() {
		if(m_obj) m_obj->release();
	}	
	
	virtual void fire() {
		if(m_obj && m_mf)
			(m_obj->*m_mf)();
	}
};


/////////////////////////
// Inline callback_event implementation

// 'obj' is the target object having a member 
// function 'mf' accepting 'arg' 
template <class T, class A>
inline callback_event<T, A>::callback_event(T* obj, void (T::*mf)(A *a), A* arg)
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
inline callback<T, A>::callback(T* obj, void (T::*mf)(A *a), A* arg)
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
