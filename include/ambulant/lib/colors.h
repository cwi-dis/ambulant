
/* 
 * @$Id$ 
 */

#ifndef AMBULANT_LIB_COLORS_H
#define AMBULANT_LIB_COLORS_H

#include "ambulant/lib/basic_types.h"
#include <stdlib.h>

namespace ambulant {

namespace lib {

typedef uint32 color_t;

extern bool is_color(const char *name);

extern color_t to_color(const char *name);

inline uchar redc(color_t rgb) {return uchar(rgb & 0xFF);}
inline uchar greenc(color_t rgb) {return uchar((rgb >> 8) & 0xFF);}
inline uchar bluec(color_t rgb) {return uchar((rgb >> 16) & 0xFF);}

template <typename T>
inline color_t to_color(T r, T g, T b)
	{return color_t(uchar(r)) | color_t(uchar(g) << 8) | color_t(uchar(b) << 16);}

// color_encoding classification
struct color_encoding {
	int size;
	int red_shift;
	int red_mask;
	int green_shift;
	int green_mask;
	int blue_shift;
	int blue_mask;
};

struct color_quad {
	uchar b;
	uchar g;
	uchar r;
	uchar a;
};

struct color_trible {
	uchar b;
	uchar g;
	uchar r;

	color_trible() : b(0), g(0), r(0) {}
	
	color_trible(uchar _r, uchar _g, uchar _b) : b(_b), g(_g), r(_r) {}
	
	template <typename T>
	color_trible(T _r, T _g, T _b) : b(uchar(_b)), g(uchar(_g)), r(uchar(_r)) {}
	
	color_trible(color_t rgb) : b(bluec(rgb)), g(greenc(rgb)), r(redc(rgb)) {}
	
	color_trible(const color_quad& q) : b(q.b), g(q.g), r(q.r) {}
	
	uchar blue() const { return b;}
	uchar green() const { return g;}
	uchar red() const { return r;}

	template <typename T>
	void blue(T _b) { b = uchar(_b);}
	
	template <typename T>
	void green(T _g) { g = uchar(_g);}
	
	template <typename T>
	void red(T _r) { r = uchar(_r);}
	
	bool operator==(color_trible o) const {
		return b == o.b && g == o.g && r == o.r;
	}
	
	bool operator!=(color_trible o) const {
		return b != o.b || g != o.g || r != o.r;
	}
	
	operator color_t () { return to_color(r, g, b);}
	
	// traits
	static color_encoding& get_encoding() { 
		static color_encoding e = {24, 0, 8, 8, 8, 16, 8};
		return e;
	}
	static size_t get_bits_size() { return 24;} 
};

} // namespace lib
 
} // namespace ambulant

#include <ostream>
inline std::ostream& operator<<(std::ostream& os, const ambulant::lib::color_trible& t) { 
	return os << '(' << int(t.r) << ", " << int(t.g) << ", " << int(t.b)  << ')';
}

#endif // AMBULANT_LIB_COLORS_H
