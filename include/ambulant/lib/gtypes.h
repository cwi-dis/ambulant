
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

/////////////////////////////
// Defines basic objects
// point, size, rect
/////////////////////////////

 
#ifndef AMBULANT_LIB_GTYPES_H
#define AMBULANT_LIB_GTYPES_H

#include "logger.h"

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
	
	basic_point<T> operator-() { 
		return basic_point<T>(-x, -y);
	}
};

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

// a basic_rect includes the points (x, y): 
// where x in [x, x+w) and y in [y, y+h)
// if w == 0 or h == 0 the basic_rect is empty

template <class T, class S = T >
class basic_rect {
  public:
	T x, y;
	S w, h;
	
	basic_rect(S _w, S _h) 
	:	x(0), y(0), w(_w), h(_h) {}
	
	basic_rect(T _x, T _y, S _w, S _h) 
	:	x(_x), y(_y), w(_w), h(_h) {}
		 
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
	
	basic_point<T> origin() const {
		return basic_point<T>(x, y);
	}
	
	basic_size<S> size() const {
		return basic_size<S>(w, h);
	}
	
	void translate(const basic_point<T>& p) {
		x += p.x; y += p.y;
	}
	
	void operator+=(basic_point<T> p) {
		translate(p);
	}
	
	void operator-=(basic_point<T> p) {
		translate(-p);
	}
		
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
	
	basic_rect<T, S> operator&(const basic_rect<T, S>& r) const {
		// return intersection
		basic_rect<T, S> l(*this); 
		l &= r;
		return l;
	}
	
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


/////////////////
// screen_rect

// A special case rectangle that uses variable naming 
// that depends on axis-orientation.
// Assumes the y axis is reflected and vertival. 
// valid rect cond: right>=left and bottom>=top;

// A screen_rect includes the points (x, y): 
// where x in [left, right) and y in [top, bottom)
// if w == 0 or h == 0 the basic_rect is empty

// Note: for convenience and by tradition
// left, top, right, bottom are public.
// This means that when set directly 
// this class may not represent a valid rect
// (use valid() member function to check this).

template <class T>
class screen_rect {
  public:
	T left, top, right, bottom;
		
	screen_rect() 
	:	left(0), top(0), right(0), bottom(0) {}
	
	screen_rect(T l, T t, T r, T b) 
	:	left(l), top(t), right(r), bottom(b) {}
	
	screen_rect(const screen_rect<T>& o) 
	:	left(o.left), top(o.top), right(o.right), bottom(o.bottom) {}
	
	template <typename S>
	screen_rect(const basic_point<T>& p, const basic_size<S>& s) 
	:	left(p.x), top(p.y), right(p.x+s.w), bottom(p.y+s.h) {}
	
	template <typename S>
	screen_rect(const basic_rect<T, S>& r) 
	:	left(r.x), top(r.y), right(r.x+r.w), bottom(r.y+r.h) {}
	
	void set_coord(T l, T t, T r, T b) {
		left = l; top = t; right = r; bottom = b;
	}
	
	template <typename S>
	void set_coord(const basic_rect<T, S>& r) {
		left = r.x; top = r.y; right = r.x+r.w; bottom = r.y+r.h;
	}

	bool valid() const {
		return right>=left && bottom>=top;
	}
	
	bool empty() const {
		return right<=left || bottom<=top;
	}
	
	basic_point<T> left_top() const {
		return basic_point<T>(left, top);
	}
	
	basic_point<T> right_bottom() const {
		return basic_point<T>(right, bottom);
	}
	
	basic_size<T> size() const {
		return basic_size<T>(lmax(0,right-left), lmax(0,bottom-top));
	}

	void translate(const basic_point<T>& p) {
		left += p.x; right += p.x;
		top += p.y; bottom += p.y;
	}
	
	void fix() {
		set_coord(lmin(left, right), lmin(top, bottom), lmax(left, right), lmax(top, bottom));
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
		return (x >= left ) && (x < right) && (y >= top ) && (y < bottom);
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
  
} // namespace lib
 
} // namespace ambulant

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

template<class T>
inline std::ostream& operator<<(std::ostream& os, const ambulant::lib::screen_rect<T>& r) { 
	return os << '(' << r.left << ", " << r.top << ", " << r.right << ", " << r.bottom << ')';
}

#endif // AMBULANT_LIB_GTYPES_H
