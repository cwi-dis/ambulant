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


// This class is useful by itself as an envelope 
// controlling the lifetime of its content,
// but can serve also as a sample for ref_counted classes.
// This is the self-destroying version of ref counted objects.
template<class T>
class sd_envelope : public ref_counted_obj {
  public:
	sd_envelope(T* value) : m_value(value) {}
	~sd_envelope() { delete m_value;}
	T* get_value() { return m_value;}
  private:
	T* m_value;
	atomic_count m_refcount;
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
