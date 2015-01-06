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

#ifndef AMBULANT_SMIL2_ANIMATE_F_H
#define AMBULANT_SMIL2_ANIMATE_F_H

#include "ambulant/config/config.h"
#include "ambulant/common/region_dim.h"
#include "ambulant/lib/gtypes.h"
#include <cmath>
#include <map>
#include <vector>
#include <cassert>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace ambulant {

namespace smil2 {

// Base class for simple dur animation functions
// Implements time manupulations for the hierarchy
//
class simple_animation_f {
  public:
	typedef long time_type;

	simple_animation_f() : m_d(0), m_accelerate(0.0), m_decelerate(0.0) {}

	time_type dur() const {
		return m_d;
	}

	time_type manipulated(time_type t) const {
		return accelerate(auto_reverse(t));
	}

	void set_auto_reverse(bool b) { m_auto_reverse = b;}
	void set_accelerate(double a, double d) { m_accelerate = a; m_decelerate = d;}

  protected:
	// a simple_animation_f is defined for the interval [0,d]
	time_type m_d;

  private:
	// t in [0, 2*d]
	time_type auto_reverse(time_type t) const {
		if(!m_auto_reverse) {
			t = (t<0)?0:(t>m_d?m_d:t);
			return t;
		}
		time_type d = 2*m_d;
		t = (t<0)?0:(t>d?d:t);
		return (t<=m_d)?t:(m_d - (t-m_d));
	}

	// t in [0, dur]
	// to preserve duration max speed m should be:
	// d = accTriangle + constRectangle + decTriangle = a*d*m/2 + (d-b*d-a*d)*m + b*d*m/2
	// therefore max speed m = 1 / (1 - a/2 - b/2)
	time_type accelerate(time_type t) const {
		t = (t<0)?0:(t>m_d?m_d:t);
		double a = m_accelerate;
		double b = m_decelerate;
		if(a == 0.0 && b == 0.0) return t;
		double d = m_d;
		double ad = a*d;
		double bd = b*d;
		double t2 = t*t;
		double dt2 = (d-t)*(d-t);
		double m = 1.0/(1.0 - 0.5*a - 0.5*b);
		double tp = 0.0;
		if(t<=ad)
			tp = 0.5*m*t2/ad;
		else if(t>=a*d && t<=(d-bd))
			tp = 0.5*m*ad + (t-ad)*m;
		else if(t>=(d-bd) && t<=d)
			tp = d - 0.5*m*dt2/bd;
		assert(tp>=0.0 && tp<=d);
		return time_type(floor(tp+0.5));
	}


	// A simple function is defined for the interval [0, d]
	double m_accelerate;
	double m_decelerate;
	bool m_auto_reverse;
};
    
    // Distance for scalars
    template <class T>
    double dist(const T& v1, const T& v2) {
        return std::max(v1, v2) - std::min(v1, v2);
    }
    
    // Distance specialization for common::region_dim
    template <>
    inline double dist(const ambulant::common::region_dim& rd1, const ambulant::common::region_dim& rd2) {
        if(rd1.relative())
            return std::max(rd1.get_as_dbl(), rd2.get_as_dbl()) - std::min(rd1.get_as_dbl(), rd2.get_as_dbl());
        else if(rd1.absolute())
            return std::max(rd1.get_as_int(), rd2.get_as_int()) - std::min(rd1.get_as_int(), rd2.get_as_int());
        return 0;
    }
    
    template <>
    inline double dist(const ambulant::common::region_dim_spec& rds1, const ambulant::common::region_dim_spec& rds2) {
        double dx = dist(rds1.left, rds2.left);
        double dy = dist(rds1.top, rds2.top);
        double dw = dist(rds1.width, rds2.width);
        double dh = dist(rds1.height, rds2.height);
        return std::max(std::max(dx, dy), std::max(dw, dh));
    }
    
    // Distance specialization for lib::point
    template <>
    double dist(const lib::point& p1, const lib::point& p2) {
        double dx = double(p2.x - p1.x);
        double dy = double(p2.y - p1.y);
        return ::sqrt(dx*dx + dy*dy);
    }
    
    // Distance specialization for lib::color_t
    template <>
    double dist(const lib::color_t& c1, const lib::color_t& c2) {
        double dr = double(lib::redc(c2) - lib::redc(c1));
        double dg = double(lib::greenc(c2) - lib::greenc(c1));
        double db = double(lib::bluec(c2) - lib::bluec(c1));
        return ::sqrt(dr*dr + dg*dg + db*db);
    }
    

// Simple duration animation function for continues attributes
// The attribute type is T
// T must define operator minus and multiplication/division with an int or double
// T must have a copy constructor
//
template<class T>
class linear_map_f : public simple_animation_f {
  public:
	typedef T value_type;
	typedef std::map<time_type, value_type> map_type;
	typedef typename map_type::const_iterator const_map_iterator;

	value_type at(time_type t) const {
		t = manipulated(t);
		const_map_iterator eit = m_ktv.upper_bound(t);
		const_map_iterator bit = eit;bit--;
		if(eit == m_ktv.end()) return (*bit).second;
		time_type dt = t - (*bit).first;
		time_type dd = (*eit).first - (*bit).first;
		value_type v1 = (*bit).second;
		value_type v2 = (*eit).second;
		return v1 + ((v2-v1)*dt)/dd;
	}

	value_type at(time_type t, const value_type& u) const {
		return at(t);
	}

	time_type dur() const {
		assert(m_d == (*m_ktv.rbegin()).first - (*m_ktv.begin()).first);
		return m_d;
	}

	map_type& get_time_values_map() { return m_ktv;}

	void init(time_type duration, T val) {
		m_d = duration;
		m_ktv.clear();
		m_ktv[0]=val;
		m_ktv[duration]=val;
	}

	void init(time_type duration, T from, T to) {
		m_d = duration;
		m_ktv.clear();
		m_ktv[0]=from;
		m_ktv[duration]=to;
	}

	void init(time_type duration, const std::vector<T>& vals) {
		assert(!vals.empty());
		m_d = duration;
		int n = int(vals.size());
		if(n==1) {
			init(duration, vals[0]);
		} else if(n==2) {
			init(duration, vals[0], vals[1]);
		} else {
			m_ktv.clear();
			time_type to = 0;
			for(int i=0;i<n;i++, to+=duration)
				m_ktv[to/(n-1)] = vals[i];
		}
	}

	void init(time_type duration, const std::vector<T>& vals, const std::vector<double>& keyTimes) {
		assert(!vals.empty());
		m_d = duration;
		int n = int(vals.size());
		if(n==1) {
			init(duration, vals[0]);
		} else if(n==2) {
			init(duration, vals[0], vals[1]);
		} else {
			// assumes keyTimes have been verified and thus
			// the following assertions evaluate to true
			assert(keyTimes.front() == 0.0);
			assert(keyTimes.back() == 1.0);
			assert(vals.size() == keyTimes.size());
			for(int j=1;j<n;j++) assert(keyTimes[j]>keyTimes[j-1]);
			m_ktv.clear();
			for(int i=0;i<n;i++) {
				time_type t = time_type(floor(keyTimes[i]*duration + 0.5));
				m_ktv[t] = vals[i];
			}
		}
	}

	void paced_init(time_type duration, const std::vector<T>& vals) {
		assert(!vals.empty());
		m_d = duration;
		int n = int(vals.size());
		if(n==1) {
			init(duration, vals[0]);
		} else if(n==2) {
			init(duration, vals[0], vals[1]);
		} else {
			m_ktv.clear();
			double length = 0.0;
			for(int i1=1;i1<n;i1++) length += dist(vals[i1-1], vals[i1]);
			double dl = 0;
			m_ktv[0] = vals[0];
			for(int i2=1;i2<n;i2++) {
				dl += dist(vals[i2-1], vals[i2]);
				time_type t = time_type(::floor(0.5+duration*dl/length));
				m_ktv[t] = vals[i2];
			}
		}
	}

  private:
	map_type m_ktv;
};

// Simple duration animation function for discrete attributes
// The attribute type is T
// T must have a copy constructor
//
template<class T>
class discrete_map_f : public simple_animation_f {
  public:
	typedef T value_type;
	typedef std::map<time_type, value_type> map_type;
	typedef typename map_type::const_iterator const_map_iterator;

	value_type at(time_type t) const {
		assert(!m_ktv.empty());
		t = manipulated(t);
		const_map_iterator eit = m_ktv.upper_bound(t);
		const_map_iterator bit = eit;bit--;
		return (*bit).second;
	}

	value_type at(time_type t, const value_type& u) const {
		return at(t);
	}

	map_type& get_time_values_map() { return m_ktv;}

	void init(time_type duration, T val) {
		m_d = duration;
		m_ktv.clear();
		m_ktv[0]=val;
	}

	void init(time_type duration, T from, T to) {
		m_d = duration;
		m_ktv.clear();
		m_ktv[0]=from;
		m_ktv[duration/2]=to;
	}

	void init(time_type duration, const std::vector<T>& vals) {
		assert(!vals.empty());
		m_d = duration;
		int n = int(vals.size());
		if(n==1) {
			init(duration, vals[0]);
		} else if(n==2) {
			init(duration, vals[0], vals[1]);
		} else {
			m_ktv.clear();
			time_type to = 0;
			for(int i=0;i<n;i++, to+=duration)
				m_ktv[to/n] = vals[i];
		}
	}

	void init(time_type duration, const std::vector<T>& vals, const std::vector<double>& keyTimes) {
		assert(!vals.empty());
		m_d = duration;
		int n = int(vals.size());
		// assumes keyTimes have been verified and thus
		// the following assertions evaluate to true
		assert(keyTimes.front() == 0.0);
		assert(keyTimes.back()<=1.0);
		assert(vals.size() == keyTimes.size());
		for(int j=1;j<n;j++) assert(keyTimes[j]>keyTimes[j-1]);
		if(n==1) {
			init(duration, vals[0]);
		} else if(n==2) {
			m_ktv.clear();
			m_ktv[0]= vals[0];
			time_type t = time_type(floor(keyTimes[1]*duration + 0.5));
			m_ktv[t] = vals[1];
		} else {
			m_ktv.clear();
			for(int i=0;i<n;i++) {
				time_type t = time_type(floor(keyTimes[i]*duration + 0.5));
				m_ktv[t] = vals[i];
			}
		}
	}

	void paced_init(long duration, const std::vector<T>& vals) {
		init(duration, vals);
	}

  private:
	map_type m_ktv;
};

// Simple duration animate function for "to" aninations
template<class T>
class underlying_to_f : public simple_animation_f {
  public:
	typedef T value_type;

	value_type at(time_type t) const {
		return at(t, value_type());
	}

	value_type at(time_type t, const T& u) const {
		t = manipulated(t);
		return (u*(m_d-t))/m_d + (m_v*t)/m_d;
	}

	void init(time_type duration, value_type v) {
		m_d = duration; m_v = v;
	}

  private:
	value_type m_v;
};

//
// Lifetime animation function
//
// Template arg F : simple duration animate function
// F requirements:
// F should define the types: time_type and value_type
// F should implement:
// value_type at(time_type t) const;
// value_type at(time_type t, const value_type& u) const;
//
template <class F>
class animate_f {
  public:
	typedef typename F::time_type time_type;
	typedef typename F::value_type value_type;

	animate_f(const F& f, time_type sd, time_type ad, bool cum = false)
	:	m_f(f), m_sd(sd), m_ad(ad), m_cum(cum) {
		// XXXJACK Added this clamping of simple duration to active duration, but I'm not sure it's
		// correct. Symptom was bug #1763572: stack overflow if sd is indefinite.
		if (m_sd > m_ad) m_sd = m_ad;
	}

	value_type at(time_type t) const {
		if(t<=m_ad) return m_cum?(m_f.at(m_sd)*(t/m_sd) + m_f.at(t%m_sd)):m_f.at(t%m_sd);
		if((m_ad%m_sd) != 0) return this->at(m_ad);
		assert((m_ad%m_sd) == 0);
		return m_cum?m_f.at(m_sd)*(m_ad/m_sd):m_f.at(m_sd);
	}

	value_type at(time_type t, const value_type& u) const {
		if(t<m_ad) return m_cum?(m_f.at(m_sd, u)*(t/m_sd) + m_f.at(t%m_sd, u)):m_f.at(t%m_sd, u);
		if((m_ad%m_sd) != 0) return this->at(m_ad, u);
		assert((m_ad%m_sd) == 0);
		return m_cum?m_f.at(m_sd, u)*(m_ad/m_sd):m_f.at(m_sd, u);
	}

#if 0
	// Unused, and I don't know what they should return.
	bool set_cumulative(bool c) { m_cum = c;}
	bool update_ad(time_type ad) { m_ad = ad;}
	bool update_sd(time_type sd) { m_sd = sd;}
#endif

	const F& m_f;
	time_type m_sd;
	time_type m_ad;
	bool m_cum;
};

enum calc_mode_t {cm_linear, cm_discrete, cm_paced, cm_spline};
#if 0
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
#endif

} // namespace smil2

} // namespace ambulant

#endif // AMBULANT_SMIL2_ANIMATE_F_H
