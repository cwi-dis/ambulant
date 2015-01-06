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
 * Ref-counted mechanism implementation.
 *
 */

#ifndef AMBULANT_LIB_REFCOUNT_H
#define AMBULANT_LIB_REFCOUNT_H

#include "ambulant/config/config.h"

// assert
#include <cassert>

#include "ambulant/lib/mtsync.h"

// reference couting should be atomic
#define USE_REF_COUNT_SEMAPHORE

namespace ambulant {

namespace lib {


/// Interface for reference counted objects.
class AMBULANTAPI ref_counted {
  public:
	virtual ~ref_counted(){}

	/// Increment reference count.
	virtual long add_ref() = 0;

	/// Decrement reference count.
	/// Decrements refcount and frees object when refcount goes to 0.
	/// Returns the new refcount.
	virtual long release() = 0;

	/// Return current reference count.
	virtual long get_ref_count() const = 0;
};

/// An atomic counter.
/// Template argument T exposes a critical section interface.
template <class T>
class basic_atomic_count {
  public:
	/// Construct with given initial value.
	explicit basic_atomic_count(long v): m_value(v){}

	/// Increment the counter.
	long operator++() {
#ifdef	USE_REF_COUNT_SEMAPHORE
		m_cs.enter();
		long rv = ++m_value;
		m_cs.leave();
		return rv;;
#else //USE_REF_COUNT_SEMAPHORE
		return ++m_value;
#endif//USE_REF_COUNT_SEMAPHORE
	}

	/// Decrement the counter.
	long operator--() {
#ifdef	USE_REF_COUNT_SEMAPHORE
		m_cs.enter();
		long rv = --m_value;
		m_cs.leave();
		return rv;
#else //USE_REF_COUNT_SEMAPHORE
		return --m_value;
#endif//USE_REF_COUNT_SEMAPHORE
	}

	/// Return the current value of the counter.
	operator long() const {
#ifdef	USE_REF_COUNT_SEMAPHORE
		const_cast<basic_atomic_count*>(this)->m_cs.enter();
		const long rv = m_value;
		const_cast<basic_atomic_count*>(this)->m_cs.leave();
		return rv;
#else //USE_REF_COUNT_SEMAPHORE
		return m_value;
#endif//USE_REF_COUNT_SEMAPHORE
	}

  private:
	basic_atomic_count(basic_atomic_count const &);
	basic_atomic_count & operator=(basic_atomic_count const &);
	long m_value;
#ifdef	USE_REF_COUNT_SEMAPHORE
	T m_cs;
#endif//USE_REF_COUNT_SEMAPHORE
};

/// An atomic counter using the standard critical_section for locking.
typedef AMBULANTAPI basic_atomic_count<critical_section> atomic_count;

/// An implementation of the ref_counted interface.
/// The ref_counted_obj class maybe used as a mixin for classes
/// implementing ref_counted objects.
/// For example:
/// class active_player : public ref_counted_obj {
///		....
///		No code related with ref counting.
///		Even the m_refcount doesn't have
///		to be initialized in the constructor of active_player
///		since the base ref_counted_obj does this by default.
///		....
/// };
class AMBULANTAPI ref_counted_obj : virtual public ref_counted {
  public:
	ref_counted_obj()
	:	m_refcount(1) {}

	virtual ~ref_counted_obj() {
		assert(m_refcount == 0);
	}

	long add_ref() {return ++m_refcount;}

	long release() {
		if(--m_refcount == 0){
			delete this;
			return 0;
		}
		return m_refcount;
	}
	long get_ref_count() const {return m_refcount;}

  protected:

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

	atomic_count m_refcount;	///< The reference count.

#ifdef _MSC_VER
#pragma warning(pop)
#endif

};

/// A ref_counted wrapper around a normal objects.
/// Converts normal objects to a ref_counted objects.
/// The cost is that you have to get the wrapped object
/// using get_ptr().
/// As all ref_counted objects and this one
/// is created with the operator new.
template<class T>
class auto_ref : public ref_counted_obj {
  public:
	/// Construct for a given object.
	auto_ref(T* ptr = 0) : m_ptr(ptr) {}
	~auto_ref() { delete m_ptr;}

	/// Return the underlying object.
	T* get_ptr() { return m_ptr;}
	/// Replace the underlying object.
	/// The old object, if any, is deleted.
	void set_ptr(T* ptr = 0) {
		if(ptr != m_ptr) delete m_ptr;
		m_ptr = ptr;
	}
  private:
	T* m_ptr;
};

/// Save release idiom:
/// p = release(p);
/// if p is deleted then p == 0.
template <class T>
static T* release(T *p) {
	if(!p) return p;
	return p->release()?p:0;
}

} // namespace lib

} // namespace ambulant

#endif // AMBULANT_LIB_REFCOUNT_H
