/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
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
class ref_counted_obj : virtual public ref_counted {
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
