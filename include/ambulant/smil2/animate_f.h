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

#ifndef AMBULANT_SMIL2_ANIMATE_F_H
#define AMBULANT_SMIL2_ANIMATE_F_H

#include "ambulant/config/config.h"
#include <cmath>
#include <map>

#ifndef AMBULANT_PLATFORM_WIN32_WCE_3
#include <cassert>
#endif

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace ambulant {

namespace smil2 {

// Simple duration animation function for continues attributes 
// The attribute type is T
// T must define operator minus and multiplication/division with an int or double 
// T must have a copy constructor
//
template<class T>
class linear_map_f {
  public:
	typedef long time_type;
	typedef T value_type;
	typedef std::map<time_type, value_type> map_type;
	
	value_type operator()(time_type t) const {
		return at(t);
	}
	
	value_type at(time_type t) const {
		assert(!m_ktv.empty());
		time_type d = dur();
		t = (t<0)?0:(t>d?d:t);	
		map_type::const_iterator eit = m_ktv.upper_bound(t);
		map_type::const_iterator bit = eit;bit--;
		if(eit == m_ktv.end()) return (*bit).second;
		time_type dt = t - (*bit).first;
		time_type dd = (*eit).first - (*bit).first;
		value_type v1 = (*bit).second;
		value_type v2 = (*eit).second;
		return v1 + (dt*(v2-v1))/dd;
	}
	
	time_type dur() const {
		assert(!m_ktv.empty());
		return (*m_ktv.rbegin()).first - (*m_ktv.begin()).first;
	}
	
	map_type& get_time_values_map() { return m_ktv;}

  private:	
	map_type m_ktv;	
};

// Simple duration animation function for discrete attributes 
// The attribute type is T
// T must have a copy constructor
//
template<class T>
class discrete_map_f {
  public:
	typedef long time_type;
	typedef T value_type;
	typedef std::map<time_type, value_type> map_type;
	
	value_type operator()(time_type t) const {
		return at(t);
	}
	
	value_type at(time_type t) const {
		assert(!m_ktv.empty());
		time_type d = dur();
		t = (t<0)?0:(t>d?d:t);	
		map_type::const_iterator eit = m_ktv.upper_bound(t);
		map_type::const_iterator bit = eit;bit--;
		return (*bit).second;
	}
	
	time_type dur() const {
		assert(!m_ktv.empty());
		return (*m_ktv.rbegin()).first - (*m_ktv.begin()).first;
	}
		
	map_type& get_time_values_map() { return m_ktv;}

  private:	
	map_type m_ktv;	
};

// Simple duration animate function for "to" aninations
template<class T, class C>
class to_f {
  public:
	typedef long time_type;
	typedef T value_type;
	typedef C context_type;
	
	to_f(time_type dur, value_type v, context_type *c) 
	:	m_d(dur), m_v(v), m_c(c) {}

	value_type operator()(time_type t) const {
		return at(t);
	}
	
	value_type at(time_type t) const {
		t = (t<0)?0:(t>m_d?m_d:t);	
		value_type u = m_c?m_c->underlying():value_type();
		return (u*(m_d-t))/m_d + (t*m_v)/m_d;
	}
	
	time_type dur() const {
		return m_d;
	}
	
  private:	
	value_type m_v;
	time_type m_d;
	context_type *m_c;
};

//
// Lifetime animation function
//
// Template arg F : simple duration animate function
// F requirements:
// F should define the types: time_type and value_type
// F should implement:
// value_type at(time_type t) const;
// time_type dur() const;
//
template <class F>
class animate_f {
  public:
	typedef typename F::time_type time_type;
	typedef typename F::value_type value_type;
	
	animate_f(const F& f, time_type ad, bool sum = false)
	:	m_f(f), m_ad(ad), m_sum(sum) {}
	
	value_type operator()(time_type t) const {
		return at(t);
	}	
	
	value_type at(time_type t) const {
		time_type d = m_f.dur();
		if(t<=m_ad) return m_sum?((t/d)*m_f.at(d) + m_f.at(t%d)):m_f.at(t%d);
		if((m_ad%d) != 0) return this->at(m_ad);
		assert((m_ad%d) == 0);
		return m_sum?(m_ad/d)*m_f.at(d):m_f.at(d);
	}
	
	bool set_cumulative(bool c) { m_sum = c;}
	bool update_ad(time_type ad) { m_ad = ad;}
	
	const F& m_f;
	time_type m_ad;
	bool m_sum;
};

enum calc_mode_t {cm_linear, cm_discrete, cm_paced, cm_spline};

template<class T>
void init_map_f(long dur, T from, T to, linear_map_f<T>& mf) {
	typedef typename linear_map_f<T>::map_type map_type;
	map_type& ktv = mf.get_time_values_map();
	ktv.clear();
	ktv[0]=from;
	ktv[dur]=to;
}

template<class T>
void init_map_f(long dur, T val, linear_map_f<T>& mf) {
	typedef typename linear_map_f<T>::map_type map_type;
	map_type& ktv = mf.get_time_values_map();
	ktv.clear();
	ktv[0]=val;
	ktv[dur]=val;
}

template<class T>
void init_map_f(long dur, T *vals, int n, linear_map_f<T>& mf) {
	assert(n>0);
	if(n==1) {
		init_map_f(dur, vals[0], mf);
	} else if(n==2) {
		init_map_f(dur, vals[0], vals[1], mf);
	} else { 
		typedef typename linear_map_f<T>::map_type map_type;
		map_type& ktv = mf.get_time_values_map();
		ktv.clear();
		time_type to = 0;
		for(int i=0;i<n;i++, to+=d)
			ktv[to/(n-1)] = vals[i];
	}
}

// Type T should define
// double dist(const T& v1, const T& v2)
template<class T>
void init_map_f_paced(long dur, T *vals, int n, linear_map_f<T>& mf) {
	assert(n>0);
	if(n==1) {
		init_map_f(dur, vals[0], mf);
	} else if(n==2) {
		init_map_f(dur, vals[0], vals[1], mf);
	} else { 
		typedef typename linear_map_f<T>::map_type map_type;
		map_type& ktv = mf.get_time_values_map();
		ktv.clear();
		double length = 0.0;
		for(int i=1;i<n;i++) length += dist(vals[i-1], vals[i]);
		double dl = 0;
		ktv[0] = vals[0];
		for(int i=1;i<n;i++) {
			dl += dist(vals[i-1], vals[i]);
			time_type t = time_type(::floor(0.5+d*dl/length));
			ktv[t] = vals[i];
		}		
	}
}

void create_bezier_map(double *e, std::map<double, double>& gr) {
	const int n = 20;
	double step = 1.0/double(n);
	double s = 0.0;
	for(int i=0;i<=n;i++) {
		double sc = 1.0-s;
		double b = 3.0*sc*sc*s;
		double c = 3.0*sc*s*s;
		double d = s*s*s;
		double t = b*e[0] + c*e[2] + d;
		double tp = b*e[1] + c*e[3] + d;
		gr[t] = tp;
		s = s + step;
	}
}

// Distance for scalars
template <class T>
double dist(const T& v1, const T& v2) {
	return std::max(v1, v2) - std::min(v1, v2); 
}

} // namespace smil2
 
} // namespace ambulant

#endif // AMBULANT_SMIL2_ANIMATE_F_H
