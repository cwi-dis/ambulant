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

// Release function for not self-destroying objects.
// Usage idiom:
// ptr_to_ref_counted = lib::release(ptr_to_ref_counted);
template <class T>
static T* release(T *p) {
	p->release();
	if(p->get_ref_count() == 0) {
		delete p;
		return 0;
	}
	return p;
}


class null_critical_section {
  public:
	void enter() {}
	void leave() {}
};


// Template argument T exposes a critical section interface.
// Boost libraries have a similar construct we may choose to use at some point.
template <class T = null_critical_section >
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


typedef basic_atomic_count<null_critical_section> atomic_count;


// This class is useful by itself as an envelope 
// controlling the lifetime of its content,
// but can serve also as a sample for ref_counted classes.
// This is a self-destroying version.
// See next one for a not self-destroying version.
// Both adhere to a well defined (but diff) usage contract.
template<class T>
class sd_envelope : public ref_counted {
  public:
	sd_envelope(T* value) : m_value(value), m_refcount(1){}
	
	T* get_value() { return m_value;}

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
	// force client to use release
	~sd_envelope() { delete m_value;}
	
  private:
	T* m_value;
	atomic_count m_refcount;
};

template<class T>
class envelope : public ref_counted {
  public:
	envelope(T* value) : m_value(value), m_refcount(1){}
	
	~envelope() { delete m_value;}
	
	T* get_value() { return m_value;}

	long add_ref() { return ++m_refcount;}

	long release() { return --m_refcount;}

	long get_ref_count() const { return m_refcount;}
	
  private:
	T* m_value;
	atomic_count m_refcount;
};


} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_REFCOUNT_H
