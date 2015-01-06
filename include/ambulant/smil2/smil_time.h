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

#ifndef AMBULANT_SMIL2_SMIL_TIME_H
#define AMBULANT_SMIL2_SMIL_TIME_H

#include "ambulant/config/config.h"
#   if __GNUC__ == 2 && __GNUC_MINOR__ <= 97
#include "ambulant/compat/limits"
#else
#include <limits>
#endif
#include <cmath>

// std::set<time_type> definition
#include <set>
#include <list>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

// This module defines a data type modeling a time value
// as defined by "The SMIL 2.0 Timing and Synchronization Module".
// The defined data type implements all the arithmetic
// operations described in that module.
// a) Min and max are automatically available due the selected
// representation of "indefinite" and "unresolved".
// b) Operations +, -, * are defined as required
// e.g. "indefinite" and "unresolved" behave as math
// infinity except that 0*indefinite is defined to be 0
// and 0*unresolved to be unresolved.

// Actually this module defines a template of such types.
// This is to allow for the underlying representations of smil_time
// to be an integral or a floating numeric.
// Exactness requires an integral numeric such as long.

// The smil_time class defines operations for smil_time instances.
// But, since it allows implicit convertions of the numeric T to smil_time
// the operations are actually available when the rhs of the
// operation is of type T.
// The class, to avoid unexpected site effects, does not allow
// the reverse:
// e.g. implicit convertions of smil_time<T> to type T.
// This conversion requires the explicit application
// of the operator().

namespace ambulant {

namespace smil2 {

template<class T>
class smil_time {

	// The underlying numeric representation of smil_time
	T m_val;

  public:

	// Create smil_time from type T
	// Allow implicit convertions of T to smil_time
	smil_time(T t = 0) : m_val(t) {}

	smil_time(const smil_time& o) : m_val(o()) {}

	// define operator += for times
	// unresolved and indefinite behave as math infinity
	smil_time& operator+=(const smil_time& rhs);

	// define operator -=
	// unresolved and indefinite behave as math infinity
	smil_time& operator-=(const smil_time& rhs);

	// define operator *=
	// unresolved and indefinite behave as math infinity
	// except that 0*indefinite is defined to be 0
	// and 0*unresolved to be unresolved.
	smil_time& operator*=(const smil_time& rhs);

	// scaling
	smil_time& operator*=(int n) {m_val*=n; return *this;}
	smil_time& operator/=(int n) {m_val/=n; return *this;}

	// negation
	smil_time& operator-() {m_val = -m_val; return *this;}

	// Time type indicators
	bool is_unresolved() const { return m_val == unresolved();}
	bool is_indefinite() const { return m_val == indefinite();}
	bool is_definite() const { return !is_indefinite() && !is_unresolved();}
	bool is_resolved() const { return m_val != unresolved();}

	// unspefied is an alias for unresolved
	bool is_specified() const { return m_val != unspecified();}
	bool is_unspecified() const { return m_val == unspecified();}

	// define comparisons
	bool operator<(const smil_time& rhs) const {return  m_val < rhs.m_val;}
	bool operator<=(const smil_time& rhs) const {return  m_val <= rhs.m_val;}
	bool operator==(const smil_time& rhs) const {return  m_val == rhs.m_val;}
	bool operator>(const smil_time& rhs) const {return  m_val > rhs.m_val;}
	bool operator>=(const smil_time& rhs) const {return  m_val >= rhs.m_val;}
	bool operator!=(const smil_time& rhs) const {return  m_val != rhs.m_val;}

	// define operations +, -, *
	smil_time operator+(const smil_time& rhs) const { smil_time t(this->m_val); t+= rhs; return t;}
	smil_time operator-(const smil_time& rhs) const { smil_time t(this->m_val); t-= rhs; return t;}
	smil_time operator*(const smil_time& rhs) const { smil_time t(this->m_val); t*= rhs; return t;}
	smil_time operator/(const smil_time& rhs) const { smil_time t(this->m_val); t/= rhs; return t;}

	smil_time i_rem(const smil_time& m) const {
		if(!m.is_definite()) return m_val;
		else if(m == 0) return smil_time(0);
		return m_val - m()*(m_val/m());
	}

	smil_time f_rem(const smil_time& m) const {
		return 0;
	}

	smil_time rem(const smil_time& m) const {
		if(std::numeric_limits<T>::round_error() == 0)
			return i_rem(m);
		return f_rem(m);
	}

	T i_mod(const smil_time& m) const {
		if(!m.is_definite()) return 0;
		else if(m == 0) return indefinite();
		return m_val/m();
	}

	T f_mod(const smil_time& m) const {
		//assert(false); // not impl
		return 0;
	}

	T mod(const smil_time& m) const {
		if(std::numeric_limits<T>::round_error() == 0)
			return i_mod(m);
		return f_mod(m);
	}
	smil_time abs() const {return m_val>=0?m_val:-m_val;}

	// Explicit convertion of smil_time<T> to T.
	// The returned value has default meaning
	// when this->is_definite().
	T operator()() const { return m_val;}

	// indefinite and unresolved instances
	static smil_time indefinite;
	static smil_time unresolved;

	// the minimum value
	static smil_time minus_infinity;

	// alias for unresolved
	static smil_time unspecified;
};

template<class T>
struct smil_interval {
	typedef smil_time<T> time_type;

	smil_interval():	begin(), end() {}

	smil_interval(const time_type& t1, const time_type& t2)
	:	begin(t1), end(t2) {}

	smil_interval(const smil_interval& rhs)
	:	begin(rhs.begin), end(rhs.end) {}


	bool is_valid() const {
		return begin.is_definite() && begin <= end;
	}

	bool is_valid_child(const smil_interval& parent) const {
		return begin < parent.end && end >= parent.begin;
	}

	bool is_zero_dur() const {
		return begin.is_definite() && begin == end;
	}

	bool is_definite() const {
		return is_valid() && end.is_definite();
	}

	void translate(const time_type& t) {
		begin += t; end += t;
	}

	void translate_to_begin() {
		end -= begin; begin = 0;
	}

	bool contains(const time_type& t) const {
		return  t==begin || (t> begin && t<end);
	}
	bool before(const time_type& t) const {
		return  end < t;
	}
	bool after(const time_type& t) const {
		return  begin > t;
	}
	bool contains(const smil_interval& other) const {
		return  contains(other.begin) && (contains(other.end) || end == other.end);
	}

	bool overlaps(const time_type& b, const time_type& e) const {
		return b<=end && e>=begin;
	}

	bool operator==(const smil_interval& rhs) const {
		return begin == rhs.begin && end == rhs.end;
	}

	bool operator!=(const smil_interval& rhs) const {
		return begin != rhs.begin || end != rhs.end;
	}

	// Returns true when this smil_interval starts before
	// or when the rhs begins, and ends before the rhs ends.
	// The rest of the comparisons are based on this.
	bool operator<(const smil_interval& rhs) const {
		return (begin <= rhs.begin && end < rhs.end);
	}

	bool operator>(const smil_interval& rhs) const {
		return rhs < *this;
	}

	bool operator<=(const smil_interval& rhs) const {
		return !(rhs < *this);
	}

	bool operator>=(const smil_interval& rhs) const {
		return !(*this < rhs);
	}

	// the begin of this smil_interval
	time_type begin;

	// the end of this smil_interval
	time_type end;

	// An instance with begin and end unresolved
	// May be used to reperesent the null smil_interval
	static smil_interval unresolved;

	// Super interval instance with begin equals
	// to zero and end unresolved
	// May be used for hyperlinking
	// (a hyperlink maybe seen as a negative begin spec
	// for the root element with respect to this interval).
	static smil_interval superinterval;
};

// times qualification
class time_node;

// A representation of a single time instance within the model.
// The time instance is initialized from the simple time of a node
// and may be converted to any other allowed by the model.
// The model allows always up-conversions.
class q_smil_time : public std::pair<const time_node*, smil_time<long> > {
  public:
	typedef long value_type;
	typedef smil_time<long> time_type;

	q_smil_time(const first_type& f, const second_type& s)
	:	std::pair<first_type, second_type>(f, s) {}

	q_smil_time(const first_type& f, long t)
	:	std::pair<first_type, second_type>(f, second_type(t)) {}

	time_type to_ancestor(const time_node *a);
	time_type to_descendent(const time_node *d);
	time_type to_node(const time_node *n);
	time_type to_doc();

	time_type as_doc_time() const;
	time_type as_node_time(const time_node *n) const;
	time_type as_time_down_to(const time_node *n) const;

	value_type as_doc_time_value() const { return as_doc_time()();}
	value_type as_node_time_value(const time_node *n) const { return as_node_time(n)();};
	value_type as_time_value_down_to(const time_node *n) const { return as_time_down_to(n)();}

	q_smil_time as_qtime_down_to(const time_node *n) const;

	q_smil_time& operator+(time_type rhs) {
		this->second += rhs;
		return *this;
	}

	q_smil_time& operator+(value_type rhs) {
		this->second += rhs;
		return *this;
	}

	q_smil_time&  operator+=(value_type rhs) {this->second += rhs; return *this;}
	q_smil_time&  operator-=(value_type rhs) {this->second -= rhs; return *this;}

  private:
	bool up();
	void down(const time_node *child);
};

template<class T>
class q_smil_interval : public std::pair<const time_node*, smil_interval<T> > {
};

// Time traits
struct time_traits {
	// The selected underlying type for time
	typedef long value_type;
	typedef unsigned long u_value_type;

	// The smil time class type for the selected value_type
	// Specifies an instance in the model
	// with respect to an implicit clock
	typedef smil_time<value_type> time_type;

	// An unambigous time instance within the model
	// Specifies explicitly the clock used.
	typedef q_smil_time qtime_type;

	// The smil interval class for the selected value_type
	// The reference clock is implicit.
	typedef smil_interval<value_type> interval_type;

	// An unambigous interval within the model
	// Specifies explicitly the clock used.
	typedef std::pair<const time_node*, interval_type> qinterval_type;

	// a set of smil time instances
	typedef std::set<time_type> time_set;
	typedef std::multiset<time_type> time_mset;

	// a list of smil time instances
	typedef std::list<time_type> time_list;

	// time units enum
	enum time_unit {tu_sec, tu_ms};

	// the selected time units
	time_unit unit() const { return tu_ms;}

	// converts secs to the selected time unit
	static time_type secs_to_time_type(double v) {
		return time_type(value_type(::floor(v*1000 + 0.5)));
	}
	// converts the selected time unit to secs
	static double time_type_to_secs(value_type v) {
		return (v/1000) + 0.001*(v % 1000);
	}
	static double time_type_to_secs(time_type v) {
		return time_type_to_secs(v());
	}
};

///////////////////////////////////
// Inline implementation

// Defines operator += for times.
// Unresolved and indefinite behave as math infinity.
template<class T> inline
smil_time<T>& smil_time<T>::operator+=(const smil_time<T>& rhs) {
	if(this->is_unresolved()) {
		return (*this);
	} else if(rhs.is_unresolved()) {
		m_val = unresolved();
	} else if(this->is_indefinite()) {
		return (*this);
	} else if(rhs.is_indefinite()) {
		m_val = indefinite();
	} else {
		m_val += rhs();
	}
	return *this;
}

// Defines operator -= for times.
// Unresolved and indefinite behave as math infinity.
template<class T> inline
smil_time<T>& smil_time<T>::operator-=(const smil_time<T>& rhs) {
	if(this->is_unresolved()) {
		return (*this);
	} else if(rhs.is_unresolved()) {
		m_val = unresolved();
	} else if(this->is_indefinite()) {
		return (*this);
	} else if(rhs.is_indefinite()) {
		m_val = indefinite();
	} else {
		m_val -= rhs();
	}
	return *this;
}

// Defines operator *= for times.
// Unresolved and indefinite behave as math infinity
// except that 0*indefinite is defined to be 0
// and 0*unresolved to be unresolved.
template<class T> inline
smil_time<T>& smil_time<T>::operator*=(const smil_time<T>& rhs) {
	if(this->is_unresolved()) {
		return (*this);
	} else if(rhs.is_unresolved()) {
		m_val = unresolved();
	} else if(this->is_indefinite()) {
		if(rhs() != 0)
			return (*this);
		else {
			m_val = 0;
		}
	} else if(rhs.is_indefinite()) {
		if(m_val != 0) {
			m_val = indefinite();
		} else {
			return (*this);
		}
	} else {
		m_val *= rhs();
	}
	return *this;
}

// indefinite instance representation
template<class T>
smil_time<T> smil_time<T>::indefinite = (std::numeric_limits<T>::infinity() != 0)?
	smil_time<T>(std::numeric_limits<T>::max()) :
	smil_time<T>(std::numeric_limits<T>::max()-1);

// unresolved instance representation
template<class T>
smil_time<T> smil_time<T>::unresolved = (std::numeric_limits<T>::infinity() != 0)?
	smil_time<T>(std::numeric_limits<T>::infinity()) :
	smil_time<T>(std::numeric_limits<T>::max());

// unspecified is an alias for unresolved
template<class T>
smil_time<T> smil_time<T>::unspecified = smil_time<T>::unresolved;



// minus_infinity instance representation
template<class T>
smil_time<T> smil_time<T>::minus_infinity =
	smil_time<T>(std::numeric_limits<T>::min());

// unresolved smil interval representation
// May be used to represent null
template<class T>
smil_interval<T> smil_interval<T>::unresolved =
	smil_interval<T>(smil_time<T>::unresolved, smil_time<T>::unresolved);

// superinterval representation
template<class T>
smil_interval<T> smil_interval<T>::superinterval =
	smil_interval<T>(smil_time<T>(0), smil_time<T>::unresolved);

} // namespace smil2

} // namespace ambulant


#include <string>
#include <stdio.h>

inline std::string repr(const ambulant::smil2::smil_time<long>& t) {
	if(t ==  ambulant::smil2::smil_time<long>::minus_infinity)
		return "minus_infinity";
	if(t.is_definite()) {
		char buf[16];
		sprintf(buf, "%ld", t());
		return buf;
	}
	return t.is_unresolved()?"unresolved":"indefinite";
}

inline std::string repr(const ambulant::smil2::smil_interval<long>& interval) {
	char buf[64];
	sprintf(buf, "[%s, %s]", repr(interval.begin).c_str(), repr(interval.end).c_str());
	return buf;
}

#endif // AMBULANT_SMIL2_SMIL_TIME_H
