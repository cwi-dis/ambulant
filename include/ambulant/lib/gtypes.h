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
	T x;	///< Coordinate.
	T y;	///< Coordinate.

	basic_point()
	:	x(0), y(0) {}

	/// Construct point from two coordinates.
	basic_point(T _x, T _y)
	:	x(_x), y(_y) {}

	/// Construct point as a copy of another point.
	basic_point(const basic_point<T>& o)
	:	x(o.x), y(o.y) {}

	bool operator==(basic_point<T> o) {	///< Operator
		return x == o.x && y == o.y;
	}

	bool operator!=(basic_point<T> o) {	///< Operator
		return x != o.x || y != o.y;
	}

	void operator+=(basic_point<T> o) {	///< Operator
		x += o.x; y += o.y;
	}

	void operator-=(basic_point<T> o) {	///< Operator
		x -= o.x; y -= o.y;
	}

	basic_point<T> operator+(basic_point<T> o) {	///< Operator
		return basic_point<T>(x + o.x, y + o.y);
	}

	basic_point<T> operator-(basic_point<T> o) {	///< Operator
		return basic_point<T>(x - o.x, y - o.y);
	}

	basic_point<T> operator-() {	///< Operator
		return basic_point<T>(-x, -y);
	}

	basic_point<T>& operator*=(int n) {	///< Operator
		x *= n; y *= n; return *this;
	}

	basic_point<T>& operator/=(int n) {	///< Operator
		x /= n; y /= n; return *this;
	}

	basic_point<T> operator*(int n) const {	///< Operator
		basic_point<T> t(*this); t*=n; return t;
	}

	basic_point<T> operator/(int n) const {	///< Operator
		basic_point<T> t(*this); t/=n; return t;
	}

};

/// A two-dimensional size.
/// S is the scalar type for the distances.
template <class S>
class basic_size {
  public:
	S w;	///< Width.
	S h;	///< Height.

	basic_size()
	:	w(0), h(0) {}

	/// Construct a size from two dimensions.
	basic_size(S _w, S _h)
	:	w(_w), h(_h) {}

	/// Construct a size from another size.
	basic_size(const basic_size<S>& o)
	:	w(o.w), h(o.h) {}

	/// True of both dimensions are zero.
	bool empty() const { return w==0 || h==0;}

	bool operator==(basic_size<S> o) {	///< Operator
		return w == o.w && h == o.h;
	}

	bool operator!=(basic_size<S> o) {	///< Operator
		return w != o.w || h != o.h;
	}

	void operator+=(basic_size<S> o) {	///< Operator
		w += o.w; h += o.h;
	}

	void operator-=(basic_size<S> o) {	///< Operator
		w -= o.w; h -= o.h;
	}

	basic_size<S> operator+(basic_size<S> o) {	///< Operator
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
	T x;	///< Left oordinate
	T y;	///< Top coordinate
	S w;	///< Width
	S h;	///< Height

	basic_rect() : x(0), y(0), w(0), h(0) {}

	/// Construct rect from another rect.
	basic_rect(const basic_rect<T, S>& o)
	:	x(o.x), y(o.y), w(o.w), h(o.h) {}

	/// Construct rect from a point and a size.
	basic_rect(const basic_point<T>& p, const basic_size<S>& s)
	:	x(p.x), y(p.y), w(s.w), h(s.h) {}

	/// Construct a rect at (0,0 with a given size.
	basic_rect(const basic_size<S>& s)
	:	x(0), y(0), w(s.w), h(s.h) {}

	/// True if the size of the rect is empty.
	bool empty() const { return w==0 || h==0;}

	bool operator==(basic_rect<T, S> o) const {	///< Operator
		return x == o.x && y == o.y && w == o.w && h == o.h;
	}

	bool operator!=(basic_rect<T, S> o) const {	///< Operator
		return x != o.x || y != o.y || w != o.w || h != o.h;
	}

	T left() const {	///< Return left coordinate.
		return x;
	}

	T top() const {	///< Return top coordinate.
		return y;
	}

	T right() const {	///< Return right coordinate (computed).
		return x+w;
	}

	T bottom() const {	///< Return bottom coordinate (computed).
		return y+h;
	}

	basic_point<T> left_top() const {	///< Return top-left point.
		return basic_point<T>(x, y);
	}

	basic_point<T> right_bottom() const {	///< Return bottom-right point (computed).
		return basic_point<T>(x+w, y+h);
	}

	basic_size<S> size() const {	///< Return size.
		return basic_size<S>(w, h);
	}

	S width() const {	///< Return width.
		return w;
	}

	S height() const {	///< Return height.
		return h;
	}

	/// Move a rectangle by a vector (specified as a point).
	void translate(const basic_point<T>& p) {
		x += p.x; y += p.y;
	}

	/// Get coordinates of a rectangle p in a base setup by this rectangle.
	basic_rect<T, S> innercoordinates(const basic_rect<T, S>& p) const {
		basic_rect<T, S> rv = p;
		rv &= *this;
		rv.translate(-left_top());
		return rv;
	}

	/// Get coordinates in outer coordinate system for a rectangle p specified
	/// in the system setup by this rectangle.
	basic_rect<T, S> outercoordinates(const basic_rect<T, S>& p) const {
		basic_rect<T, S> rv = p;
		rv.translate(left_top());
		rv &= *this;
		return rv;
	}

	void operator+=(basic_point<T> p) {	///< operator
		translate(p);
	}

	void operator-=(basic_point<T> p) {	///< operator
		translate(-p);
	}
    
	/// Intersect this rectangle by another rectangle o.
	void operator&=(const basic_rect<T, S>& o) {
		// set to intersection
		if (empty ()  || o.empty()) {
			// handle empty rects
			x = y = h = w = 0;
		} else {
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
	}
    
	/// Set this rectangle to a rectangle that also incorporates rectangle o.
	void operator|=(const basic_rect<T, S>& o) {
		// set to union
		// handle empty rects
		if (o.empty()) return;
		if (empty()) {
		  	x = o.x;
			y = o.y;
			w = o.w;
			h = o.h;
		} else {
			int x1 = lmin(x, o.x);
			int x2 = lmax(x+w, o.x + o.w);
			w = x2 - x1;
			x = x1;
            
			int y1 = lmin(y, o.y);
			int y2 = lmax(y+h, o.y + o.h);
			h = y2 - y1;
			y = y1;
		}
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

	/// Return true if this recatngle contains the given point.
	bool contains(const basic_point<T>& p) const {
		return contains(p.x, p.y);
	}

	/// Return true if this recatngle contains the point (xp, yp).
	bool contains(T xp, T yp) const {
		return (xp >= x ) && (xp < x + (T)w) && (yp >= y ) && (yp < y + (T)h);
	}

	/// Return true if this rect has the same size as a given rect.
	bool same_size(const basic_rect<T, S>& o) const {
		return w == o.w && h == o.h;
	}
};

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

// Returns the coord where the arguement 'x' is mapped using the same
// transform that mapped the 'src' argument to the 'dst' argument.
// In the following primes represent dest coordinates (x -> x_p)
// xp = ( (x_2^p-x_1^p)*(x-x_1) + (x_2 - x_1)*x_1_p )/(x_2 - x_1)
inline int
tf_x(int x, const rect *src, const rect *dst)
{
	double x1 = src->left(), x2 = src->right();
	double x1p = dst->left(), x2p = dst->right();
	double xp = ((x2p-x1p)*(x-x1) + (x2-x1)*x1p)/(x2-x1);
	return int(floor(xp+0.5));
}

// Returns the x coord mapped to the destination 'xp' using the same
// transform that mapped the 'src' argument to the 'dst' argument.
// In the following primes represent dest coordinates (x -> x_p)
// x = ((x_2 - x_1)*x^p + (x_1*x_2^p-x_2*x_1^p))/(x_2^p-x_1^p)
inline int
reverse_tf_x(int xp, const rect *src, const rect *dst)
{
	double x1 = src->left(), x2 = src->right();
	double x1p = dst->left(), x2p = dst->right();
	double x = ((x2-x1)*xp + (x1*x2p-x2*x1p))/(x2p-x1p);
	return (int)floor(x+0.5);
}

// Returns the coord where the arguement 'y' is mapped using the same
// transform that mapped the 'src' argument to the 'dst' argument.
// In the following primes represent dest coordinates (y -> y_p)
// yp = ( (y_2^p-y_1^p)*(y-y_1) + (y_2 - y_1)*y_1_p )/(y_2 - y_1)
inline int
tf_y(int y, const rect *src, const rect *dst)
{
	double y1 = src->top(), y2 = src->bottom();
	double y1p = dst->top(), y2p = dst->bottom();
	double yp = ((y2p-y1p)*(y-y1) + (y2-y1)*y1p)/(y2-y1);
	return (int)floor(yp+0.5);
}

// Returns the y coord mapped to the destination (yp) using the same
// transform that mapped src argument to dst argument.
// In the following primes represent dest coordinates (y -> y_p)
// y = ((y_2 - y_1)*y^p + (y_1*y_2^p-y_2*y_1^p))/(y_2^p-y_1^p)
inline int
reverse_tf_y(int yp, const rect *src, const rect *dst)
{
	double y1 = src->top(), y2 = src->bottom();
	double y1p = dst->top(), y2p = dst->bottom();
	double y = ((y2-y1)*yp + (y1*y2p-y2*y1p))/(y2p-y1p);
	return (int)floor(y+0.5);
}

// Returns the rect where 'psrc' is mapped using the same
// transform that mapped 'src' argument to 'dst' argument.
inline rect
transform(const rect *psrc, const rect *src, const rect *dst)
{
	int l = tf_x(psrc->left(), src, dst);
	int t = tf_y(psrc->top(), src, dst);
	rect rc(
		point(l, t),
		size(
			tf_x(psrc->right(), src, dst)-l,
			tf_y(psrc->bottom(), src, dst)-t));
	return rc;
}

// Returns the source rect mapped to the destination (pdst) using the same
// transform that mapped 'src' argument to 'dst' argument.
inline rect
reverse_transform(const rect *pdst, const rect *src, const rect *dst)
{
	int l = reverse_tf_x(pdst->left(), src, dst);
	int t = reverse_tf_y(pdst->top(), src, dst);
	rect rc(
		point(l, t),
		size(reverse_tf_x(pdst->right(), src, dst)-l,
			reverse_tf_y(pdst->bottom(), src, dst)-t));
	return rc;
}


} // namespace lib

} // namespace ambulant

#include "ambulant/lib/string_util.h"

inline std::string repr(const ambulant::lib::basic_point<int>& p) {
	std::string s;
	return s << '(' << p.x << ", " << p.y << ')';
}

inline std::string repr(const ambulant::lib::basic_size<unsigned int>& z) {
	std::string s;
	return s << '(' << z.w << ", " << z.h << ')';
}

inline std::string repr(const ambulant::lib::rect& r) {
	std::string s;
	return s << '(' << r.left() << ", " << r.top() << ", " << r.right() << ", " << r.bottom() << ')';
}

// gtypes output operators
#include <ostream>

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


#endif // AMBULANT_LIB_GTYPES_H
