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

/////////////////////////////
// Defines basic objects
// point, size, rect
/////////////////////////////

 
#ifndef AMBULANT_LIB_GTYPES_H
#define AMBULANT_LIB_GTYPES_H

#include "ambulant/config/config.h"
#include <math.h>

namespace ambulant {

namespace lib {

// bypass macros (these belong to lib/utility.h)
template<class T> inline
const T& lmax(const T& l, const T& r){
	return (l < r ? r : l);
}
template<class T> inline
const T& lmin(const T& l, const T& r){
	return (l < r ? l : r);
}

/// A two-dimensional (x, y) point.
/// T is the scalar type for the coordinates.
template <class T>
class basic_point {
  public:
	T x, y;
	
	basic_point() 
	:	x(0), y(0) {}
	
	basic_point(T _x, T _y) 
	:	x(_x), y(_y) {}
	 
	basic_point(const basic_point<T>& o) 
	:	x(o.x), y(o.y) {}
	
	bool operator==(basic_point<T> o) {
		return x == o.x && y == o.y;
	}
	
	bool operator!=(basic_point<T> o) {
		return x != o.x || y != o.y;
	}
	
	void operator+=(basic_point<T> o) {
		x += o.x; y += o.y;
	}
	
	void operator-=(basic_point<T> o) {
		x -= o.x; y -= o.y;
	}
	
	basic_point<T> operator+(basic_point<T> o) {
		return basic_point<T>(x + o.x, y + o.y);
	}
	
	basic_point<T> operator-(basic_point<T> o) {
		return basic_point<T>(x - o.x, y - o.y);
	}
	
	basic_point<T> operator-() { 
		return basic_point<T>(-x, -y);
	}
	
	basic_point<T>& operator*=(int n) {
		x *= n; y *= n; return *this;
	}
	
	basic_point<T>& operator/=(int n) {
		x /= n; y /= n; return *this;
	}
	
	basic_point<T> operator*(int n) const { 
		basic_point<T> t(*this); t*=n; return t;
	}
	
	basic_point<T> operator/(int n) const { 
		basic_point<T> t(*this); t/=n; return t;
	}
	
};

/// A two-dimensional size.
/// S is the scalar type for the distances.
template <class S>
class basic_size {
  public:
	S w, h;
	
	basic_size() 
	:	w(0), h(0) {}
	
	basic_size(S _w, S _h) 
	:	w(_w), h(_h) {}
	 
	basic_size(const basic_size<S>& o) 
	:	w(o.w), h(o.h) {}

	bool empty() const { return w==0 || h==0;}

	bool operator==(basic_size<S> o) {
		return w == o.w && h == o.h;
	}
	
	bool operator!=(basic_size<S> o) {
		return w != o.w || h != o.h;
	}
	
	void operator+=(basic_size<S> o) {
		w += o.w; h += o.h;
	}
	
	void operator-=(basic_size<S> o) {
		w -= o.w; h -= o.h;
	}
	
	basic_size<S> operator+(basic_size<S> o) {
		return basic_size<S>(w + o.w, h + o.h);
	}
};

/// A two-dimensional rectangle.
/// a basic_rect includes the points (xx, yy): 
/// where xx in [x, x+w) and yy in [y, y+h)
/// if w == 0 or h == 0 the basic_rect is empty.
template <class T, class S = T >
class basic_rect {
  public:
	T x, y;
	S w, h;
	
#if 0
	basic_rect(S _w, S _h) 
	:	x(0), y(0), w(_w), h(_h) {}
	
	basic_rect(T _x, T _y, S _w, S _h) 
	:	x(_x), y(_y), w(_w), h(_h) {}
#endif
	basic_rect() : x(0), y(0), w(0), h(0) {}
		 
	basic_rect(const basic_rect<T, S>& o) 
	:	x(o.x), y(o.y), w(o.w), h(o.h) {}
	
	basic_rect(const basic_point<T>& p, const basic_size<S>& s) 
	:	x(p.x), y(p.y), w(s.w), h(s.h) {}
	
	basic_rect(const basic_size<S>& s) 
	:	x(0), y(0), w(s.w), h(s.h) {}
	
	bool empty() const { return w==0 || h==0;}
	
	bool operator==(basic_rect<T, S> o) const {
		return x == o.x && y == o.y && w == o.w && h == o.h;
	}
	
	bool operator!=(basic_rect<T, S> o) const {
		return x != o.x || y != o.y || w != o.w || h != o.h;
	}
	
	T left() const {
		return x;
	}
	
	T top() const {
		return y;
	}
	
	T right() const {
		return x+w;
	}
	
	T bottom() const {
		return y+h;
	}
	
	basic_point<T> left_top() const {
		return basic_point<T>(x, y);
	}
	
	basic_point<T> right_bottom() const {
		return basic_point<T>(x+w, y+h);
	}
	
	basic_size<S> size() const {
		return basic_size<S>(w, h);
	}
	
	S width() const {
		return w;
	}

	S height() const {
		return h;
	}

	/// Move a rectangle by a vector (specified as a point).
	void translate(const basic_point<T>& p) {
		x += p.x; y += p.y;
	}
	
	/// Get coordinates of a rectangle p in a base setup by this rectangle.
	basic_rect<T, S> innercoordinates(const basic_rect<T, S>& p) const {
		basic_rect<T, S> rv = p;
		rv &= this;
		rv.translate(left_top());
		return rv;
	}
	
	/// Get coordinates in outer coordinate system for a rectangle p specified
	/// in the system setup by this rectangle.
	basic_rect<T, S> outercoordinates(const basic_rect<T, S>& p) const {
		basic_rect<T, S> rv = p;
		rv.translate(-left_top());
		rv &= this;
		return rv;
	}
	
	void operator+=(basic_point<T> p) {
		translate(p);
	}
	
	void operator-=(basic_point<T> p) {
		translate(-p);
	}
	
	/// Intersect this rectangle by another rectangle o.
	void operator&=(const basic_rect<T, S>& o) {
		// set to intersection
		// xxx: handle empty rect
		int x1 = lmax(x, o.x);
		int x2 = lmin(x + w, o.x + o.w);
		if(x2 < x1) w = 0;
		else w = x2 - x1;
		x = x1;
		
		int y1 = lmax(y, o.y);
		int y2 = lmin(y + h, o.y + o.h);
		if(y2 < y1) h = 0;
		else h = y2 - y1;
		y = y1;
	}
	
	/// Set this rectangle to a rectangle that also incorporates rectangle o.
	void operator|=(const basic_rect<T, S>& o) {
		// set to union
		// xxx: handle empty rect
		int x1 = lmin(x, o.x);
		int x2 = lmax(x+w, o.x + o.w);
		w = x2 - x1;
		x = x1;
		
		int y1 = lmin(y, o.y);
		int y2 = lmax(y+h, o.y + o.h);
		h = y2 - y1;
		y = y1;
	}
	
	/// Intersect two rectangles.
	basic_rect<T, S> operator&(const basic_rect<T, S>& r) const {
		// return intersection
		basic_rect<T, S> l(*this); 
		l &= r;
		return l;
	}
	
	/// Return rectangle encompassing both rectangles.
	basic_rect<T, S> operator|(const basic_rect<T, S>& r)  const {
		// return union
		basic_rect<T, S> l(*this); 
		l |= r;
		return l; 
	}
	
	bool contains(const basic_point<T>& p) const {
		return contains(p.x, p.y);
	}
	
	bool contains(T xp, T yp) const {
		return (xp >= x ) && (xp < x + w) && (yp >= y ) && (yp < y + h);
	}
	
	bool same_size(const basic_rect<T, S>& o) const {
		return w == o.w && h == o.h;
	}
};


/// A special case rectangle that uses variable naming 
/// that depends on axis-orientation.
/// Assumes the y axis is reflected and vertival. 
/// valid rect cond: right>=left and bottom>=top;
///
/// A screen_rect includes the points (x, y): 
/// where x in [left, right) and y in [top, bottom)
/// if w == 0 or h == 0 the basic_rect is empty
///
/// Note: for convenience and by tradition
/// left, top, right, bottom are public.
/// This means that when set directly 
/// this class may not represent a valid rect
/// (use valid() member function to check this).

template <class T>
class screen_rect {
  public:
	T m_left, m_top, m_right, m_bottom;
		
	screen_rect() 
	:	m_left(0), m_top(0), m_right(0), m_bottom(0) {}
	
	screen_rect(const screen_rect<T>& o) 
	:	m_left(o.m_left), m_top(o.m_top), m_right(o.m_right), m_bottom(o.m_bottom) {}
	
	screen_rect(const basic_point<T>& p, const basic_point<T>& q) 
	:	m_left(p.x), m_top(p.y), m_right(q.x), m_bottom(q.y) {}

	template <class S>
	screen_rect(const basic_point<T>& p, const basic_size<S>& s) 
	:	m_left(p.x), m_top(p.y), m_right(p.x+s.w), m_bottom(p.y+s.h) {}
	
	template <class S>
	screen_rect(const basic_rect<T, S>& r) 
	:	m_left(r.x), m_top(r.y), m_right(r.x+r.w), m_bottom(r.y+r.h) {}
	
	void set_coord(T l, T t, T r, T b) {
		m_left = l; m_top = t; m_right = r; m_bottom = b;
	}
	
	template <class S>
	void set_coord(const basic_rect<T, S>& r) {
		m_left = r.x; m_top = r.y; m_right = r.x+r.w; m_bottom = r.y+r.h;
	}
	bool valid() const {
		return m_right>=m_left && m_bottom>=m_top;
	}
	
	bool empty() const {
		return m_right<=m_left || m_bottom<=m_top;
	}
	
	T left() const {
		return m_left;
	}
	
	T top() const {
		return m_top;
	}
	
	T right() const {
		return m_right;
	}
	
	T bottom() const {
		return m_bottom;
	}
	
	basic_point<T> left_top() const {
		return basic_point<T>(m_left, m_top);
	}
	
	basic_point<T> right_bottom() const {
		return basic_point<T>(m_right, m_bottom);
	}
	
	basic_size<T> size() const {
		return basic_size<T>(lmax(0,m_right-m_left), lmax(0,m_bottom-m_top));
	}

	T width() const {
		return lmax(0,m_right-m_left);
	}

	T height() const {
		return lmax(0,m_bottom-m_top);
	}

	void translate(const basic_point<T>& p) {
		m_left += p.x; m_right += p.x;
		m_top += p.y; m_bottom += p.y;
	}

	screen_rect<T> outercoordinates(const screen_rect<T>& p) const {
		screen_rect<T> rv = p;
		rv.translate(left_top());
		rv &= *this;
		return rv;
	}
		
	screen_rect<T> innercoordinates(const screen_rect<T>& p) const {
		screen_rect<T> rv = p;
		rv &= *this;
		rv.translate(-left_top());
		return rv;
	}
		
	void fix() {
		set_coord(lmin(m_left, m_right), lmin(m_top, m_bottom), lmax(m_left, m_right), lmax(m_top, m_bottom));
	}
	
	bool operator==(const screen_rect<T> o) const {
		return m_left == o.left() && m_top == o.top() &&
			m_right == o.right() && m_bottom == o.bottom();
	}
	
	bool operator!=(const screen_rect<T> o) const {
		return !(o == (*this));
	}

	void operator+=(basic_point<T> p) {
		translate(p);
	}
	
	void operator-=(basic_point<T> p) {
		translate(-p);
	}
	
	void operator&=(const screen_rect<T>& o) {
		// set to intersection
		// re-use basic_rect implementation
		basic_rect<T> r1(left_top(), size());
		basic_rect<T> r2(o.left_top(), o.size());
		set_coord(r1 & r2);
	}

	void operator|=(const screen_rect<T>& o) {
		// set to union
		// re-use basic_rect implementation
		basic_rect<T> r1(left_top(), size());
		basic_rect<T> r2(o.left_top(), o.size());
		set_coord(r1 | r2);
	}
	
	screen_rect<T> operator&(const screen_rect<T>& r) const {
		// return intersection
		screen_rect<T> l(*this); 
		l &= r;
		return l;
	}
	
	screen_rect<T> operator|(const screen_rect<T>& r)  const {
		// return union
		screen_rect<T> l(*this); 
		l |= r;
		return l; 
	}

	bool contains(const basic_point<T>& p) const {
		return contains(p.x, p.y);
	}
	
	bool contains(T x, T y) const {
		return (x >= m_left ) && (x < m_right) && (y >= m_top ) && (y < m_bottom);
	}
	
};

// Returns the coord where the arguement 'x' is mapped using the same
// transform that mapped the 'src' argument to the 'dst' argument.
// In the following primes represent dest coordinates (x -> x_p)
// xp = ( (x_2^p-x_1^p)*(x-x_1) + (x_2 - x_1)*x_1_p )/(x_2 - x_1)
template <class T>
inline int tf_x(T x, const screen_rect<int> *src, const screen_rect<int> *dst) {
	double x1 = src->left(), x2 = src->right();
	double x1p = dst->left(), x2p = dst->right();
	double xp = ((x2p-x1p)*(x-x1) + (x2-x1)*x1p)/(x2-x1);
	return int(floor(xp+0.5));
}

// Returns the x coord mapped to the destination 'xp' using the same
// transform that mapped the 'src' argument to the 'dst' argument.
// In the following primes represent dest coordinates (x -> x_p)
// x = ((x_2 - x_1)*x^p + (x_1*x_2^p-x_2*x_1^p))/(x_2^p-x_1^p)
template <class T>
inline T reverse_tf_x(T xp, const screen_rect<T> *src, const screen_rect<T> *dst) {
	double x1 = src->left(), x2 = src->right();
	double x1p = dst->left(), x2p = dst->right();
	double x = ((x2-x1)*xp + (x1*x2p-x2*x1p))/(x2p-x1p);
	return (int)floor(x+0.5);
}

// Returns the coord where the arguement 'y' is mapped using the same
// transform that mapped the 'src' argument to the 'dst' argument.
// In the following primes represent dest coordinates (y -> y_p)
// yp = ( (y_2^p-y_1^p)*(y-y_1) + (y_2 - y_1)*y_1_p )/(y_2 - y_1)
template <class T>
inline T tf_y(T y, const screen_rect<T> *src, const screen_rect<T> *dst) {
	double y1 = src->top(), y2 = src->bottom();
	double y1p = dst->top(), y2p = dst->bottom();
	double yp = ((y2p-y1p)*(y-y1) + (y2-y1)*y1p)/(y2-y1);
	return (int)floor(yp+0.5);
}

// Returns the y coord mapped to the destination (yp) using the same
// transform that mapped src argument to dst argument.
// In the following primes represent dest coordinates (y -> y_p)
// y = ((y_2 - y_1)*y^p + (y_1*y_2^p-y_2*y_1^p))/(y_2^p-y_1^p)
template <class T>
inline T reverse_tf_y(T yp, const screen_rect<T> *src, const screen_rect<int> *dst){
	double y1 = src->top(), y2 = src->bottom();
	double y1p = dst->top(), y2p = dst->bottom();
	double y = ((y2-y1)*yp + (y1*y2p-y2*y1p))/(y2p-y1p);
	return (int)floor(y+0.5);
}

// Returns the rect where 'psrc' is mapped using the same
// transform that mapped 'src' argument to 'dst' argument.
template <class T>
inline lib::screen_rect<T> transform( const lib::screen_rect<T> *psrc, 
	const lib::screen_rect<T> *src, const lib::screen_rect<T> *dst) {
	lib::screen_rect<T> rc;
	rc.set_coord(tf_x(psrc->left(), src, dst),
		tf_y(psrc->top(), src, dst),
		tf_x(psrc->right(), src, dst),
		tf_y(psrc->bottom(), src, dst));
	return rc;
}

// Returns the source rect mapped to the destination (pdst) using the same
// transform that mapped 'src' argument to 'dst' argument.
template <class T>
inline lib::screen_rect<T> reverse_transform(const lib::screen_rect<T> *pdst, 
	const lib::screen_rect<T> *src, const lib::screen_rect<T> *dst){
	lib::screen_rect<T> rc;
	rc.set_coord(reverse_tf_x(pdst->left(), src, dst),
		reverse_tf_y(pdst->top(), src, dst),
		reverse_tf_x(pdst->right(), src, dst),
		reverse_tf_y(pdst->bottom(), src, dst));
	return rc;
}

// short names for the common cases

// int based
typedef basic_point<int> point;
typedef basic_size<unsigned int> size;
typedef basic_rect<int, unsigned int> rect;

// double based
typedef basic_point<double> dpoint;
typedef basic_size<double> dsize;
typedef basic_rect<double> drect;

// long based
typedef basic_point<long> lpoint;
typedef basic_size<unsigned long> lsize;
typedef basic_rect<long, unsigned long> lrect;
  
} // namespace lib
 
} // namespace ambulant

#include "ambulant/lib/string_util.h"

#if !defined(AMBULANT_PLATFORM_WIN32_WCE)

inline std::string repr(const ambulant::lib::basic_point<int>& p) {
	std::string s;
	return s << '(' << p.x << ", " << p.y << ')';
}

inline std::string repr(const ambulant::lib::basic_size<unsigned int>& z) {
	std::string s;
	return s << '(' << z.w << ", " << z.h << ')';
}

inline std::string repr(const ambulant::lib::screen_rect<int>& r) {
	std::string s;
	return s << '(' << r.left() << ", " << r.top() << ", " << r.right() << ", " << r.bottom() << ')';
}

#else 
inline std::string repr(const ambulant::lib::basic_point<int>& p) { return "";}
inline std::string repr(const ambulant::lib::basic_size<unsigned int>& z) { return "";}
inline std::string repr(const ambulant::lib::screen_rect<int>& r) {return "";}
#endif


#ifndef AMBULANT_NO_IOSTREAMS

// gtypes output operators
#ifndef AMBULANT_NO_OSTREAM
#include <ostream>
#else /*AMBULANT_NO_OSTREAM*/
#include <ostream.h>
#endif/*AMBULANT_NO_OSTREAM*/

template<class T>
inline std::ostream& operator<<(std::ostream& os, const ambulant::lib::basic_point<T>& p) { 
	return os << '(' << p.x << ", " << p.y << ')';
}

template<class S>
inline std::ostream& operator<<(std::ostream& os, const ambulant::lib::basic_size<S>& s) { 
	return os << '(' << s.w << ", " << s.h << ')';
}

template<class T, class S>
inline std::ostream& operator<<(std::ostream& os, const ambulant::lib::basic_rect<T, S>& r) { 
	return os << '(' << r.x << ", " << r.y << ", " << r.w << ", " << r.h << ')';
}

template<class T>
inline std::ostream& operator<<(std::ostream& os, const ambulant::lib::screen_rect<T>& r) { 
	return os << '(' << r.left() << ", " << r.top() << ", " << r.right() << ", " << r.bottom() << ')';
}

#endif // AMBULANT_NO_IOSTREAMS

#endif // AMBULANT_LIB_GTYPES_H
