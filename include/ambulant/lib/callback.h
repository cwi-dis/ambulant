/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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

#include "ambulant/config/config.h"

// a callback implements event
#include "ambulant/lib/event.h"

// callback targets may be ref counted
#include "ambulant/lib/refcount.h"

namespace ambulant {

namespace lib {

/// A structure able to hold the
/// generic callback arguments.
/// T is the target object class
/// A is the callback argument class.
template <class T, class A>
struct callback_struct {
	/// The target object.
	T *m_obj;

	/// The target member function.
	void (T::*m_mf)(A *a);

	/// The argument to be passed to the member function.
	/// This callback_struct object is the owner of the argument object.
	A *m_arg;

	/// callback_struct constructor.
	callback_struct(T* obj, void (T::*mf)(A *a), A* arg)
	:	m_obj(obj), m_mf(mf), m_arg(arg) {}
};

/// Callback with an argument that is not refcounted.
///
/// This callback becomes the owner of the argument
/// (e.g. it is responsible to delete arg)
///
/// The target object is not ref counted and
/// should exist when this fires.

template <class T, class A>
class callback_event : public event,
	protected callback_struct<T, A> {
  public:
	///  Constructor.
	/// 'obj' is the target object having a member function 'mf' accepting 'arg'.
	callback_event(T* obj, void (T::*mf)(A *a), A* arg);

	/// delete callback_event and arg.
	~callback_event();

	/// event interface implementation.
	virtual void fire();
};

/// Callback with an argument that is a builtin type.
///
/// Apropriate for built-in types arguments (int, double, etc)
/// May be used also for simple structures like point and size.
/// May be used for built-in types arguments
/// or arguments that are references or const references to objects.
template <class T, class A>
class scalar_arg_callback_event : public event {
	T *m_obj;
	void (T::*m_mf)(A a);
	A m_arg;

  public:
	/// Constructor.
	scalar_arg_callback_event(T* obj, void (T::*mf)(A a), A arg)
	:	m_obj(obj), m_mf(mf), m_arg(arg) {}

	virtual void fire() {
		if(m_obj && m_mf)
			(m_obj->*m_mf)(m_arg);
	}
};

/// Callback with two arguments that are builtin types.
///
/// Apropriate for built-in types arguments (int, double, etc)
/// May be used also for simple structures like point and size.
/// May be used for built-in types arguments
/// or arguments that are references or const references to objects.
template <class T, class A, class B>
class scalar_arg2_callback_event : public event {
	T *m_obj;
	void (T::*m_mf)(A a, B b);
	A m_a;
	B m_b;

  public:
	/// Constructor.
	scalar_arg2_callback_event(T* obj, void (T::*mf)(A, B), A a, B b)
	:	m_obj(obj), m_mf(mf), m_a(a), m_b(b) {}

	virtual void fire() {
		if(m_obj && m_mf)
			(m_obj->*m_mf)(m_a, m_b);
	}
};

/// Callback without arguments.
template <class T>
class no_arg_callback_event : public event {
  public:
	T *m_obj;	///< Object on which to do the callback.
	void (T::*m_mf)();	///< Method to call.

	/// Constructor.
	no_arg_callback_event(T* obj, void (T::*mf)())
	:	m_obj(obj), m_mf(mf) {}

	virtual void fire() {
		if(m_obj && m_mf)
			(m_obj->*m_mf)();
	}
};

/// Callback with argument that is also an event.
template <class T, class E = event >
class event_callback : public event {
	T *m_obj;
	void (T::*m_mf)(E *e);
  public:
	/// Constructor.
	event_callback(T* obj, void (T::*mf)(E *e))
	:	m_obj(obj), m_mf(mf) {}

	virtual void fire() {
		if(m_obj) (m_obj->*m_mf)(reinterpret_cast<E*>(this));
	}
};

/// Ref counted version of a callback.
///
/// This callback becomes the owner of the argument
/// (e.g. it is responsible to delete arg)
///
/// The target object is ref counted.

template <class T, class A>
class callback : public event,
	private callback_struct<T, A> {
  public:
	/// Constrructor.
	/// 'obj' is the target object having a member function 'mf' accepting 'arg'
	callback(T* obj, void (T::*mf)(A *a), A* arg);

	// deletes arg, releases target tref
	~callback();

	// event interface implementation
	virtual void fire();
};

/// Callback with an argument that is a builtin type to refcounted obj.
///
/// Apropriate for built-in types arguments (int, double, etc)
/// May be used also for simple structures like point and size.
/// May be used for built-in types arguments
/// or arguments that are references or const references to objects
template <class T, class A>
class scalar_arg_callback : public event {
	T *m_obj;
	void (T::*m_mf)(A a);
	A m_arg;

  public:
	/// Constructor.
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

/// Callback without arguments to refcounted obj.

template <class T>
class no_arg_callback : public event {
  public:
	T *m_obj;	///< Object on which to do the callback.

	void (T::*m_mf)();	///< Method to call.

	/// Constructor.
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
	delete this->m_arg;
}

// event interface implementation
template <class T, class A>
inline void callback_event<T, A>::fire() {
	if(this->m_mf != 0 && this->m_obj != 0)
		(*this->m_obj->m_mf)(this->m_arg);
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
	delete this->m_arg;
	if(this->m_obj != 0) this->m_obj->release();
}

// event interface implementation
template <class T, class A>
inline void callback<T, A>::fire() {
	if(this->m_mf != 0 && this->m_obj != 0)
		(*this->m_obj->m_mf)(this->m_arg);
}

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_CALLBACK_H
