/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

/* 
 * Ref-counted mechanism implementation.
 *
 */
 
#ifndef AMBULANT_LIB_REFCOUNT_H
#define AMBULANT_LIB_REFCOUNT_H

// assert
#include <cassert>

#include "ambulant/lib/mtsync.h"

namespace ambulant {

namespace lib {


// Interface of reference counted objects.
class ref_counted {
  public:
	virtual ~ref_counted(){}
	virtual long add_ref() = 0;
	virtual long release() = 0;
	virtual long get_ref_count() const = 0;
};

// Template argument T exposes a critical section interface.
// Boost libraries have a similar construct we may choose to use at some point.
template <class T>
class basic_atomic_count {	
  public:
	explicit basic_atomic_count(long v): m_value(v){}
	
	long operator++() {
		m_cs.enter();
		++m_value;
		m_cs.leave();
		return m_value;
	}
	
	long operator--() {
		m_cs.enter();
		--m_value;
		m_cs.leave();
		return m_value;
	}
	
	operator long() const {return m_value;}
	
  private:
	basic_atomic_count(basic_atomic_count const &);
	basic_atomic_count & operator=(basic_atomic_count const &);
	long m_value;
	T m_cs;
};

typedef basic_atomic_count<critical_section> atomic_count;


// The following class maybe used as a mixin for classes
// implementing ref_counted objects.
// For example:
// class active_player : public ref_counted_obj {
//		....
//		No code related with ref counting.
//		Even the m_refcount doesn't have
//		to be initialized in the constructor of active_player 
//		since the base ref_counted_obj does this by default.
//		....
// };
class ref_counted_obj : public ref_counted {
  public:
	ref_counted_obj()
	:	m_refcount(1) {}
	
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
	atomic_count m_refcount;
};

// A ref_counted wrapper around a normal objects.
// Converts normal objects to a ref_counted objects.
// The cost is that you have to get the wrapped object
// using get_ptr().
// As all ref_counted objects and this one
// is created with the operator new.
template<class T>
class auto_ref : public ref_counted_obj {
  public:
	auto_ref(T* ptr = 0) : m_ptr(ptr) {}
	~auto_ref() { delete m_ptr;}
	
    T* get_ptr() { return m_ptr;}
	void set_ptr(T* ptr = 0) {
		if(ptr != m_ptr) delete m_ptr;
		m_ptr = ptr;
	}
  private:
	T* m_ptr;
};

// Save release idiom: 
// p = release(p); 
// if p is deleted then p == 0.
template <class T>
static T* release(T *p) {
	if(!p) return p;
	return p->release()?p:0;
}

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_REFCOUNT_H
