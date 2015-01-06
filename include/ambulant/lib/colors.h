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

#ifndef AMBULANT_LIB_COLORS_H
#define AMBULANT_LIB_COLORS_H

#include "ambulant/config/config.h"
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

inline float redf(color_t rgb) { return float(redc(rgb) / 255.0); }
inline float greenf(color_t rgb) { return float(greenc(rgb) / 255.0); }
inline float bluef(color_t rgb) { return float(bluec(rgb) / 255.0); }

template <class T>
inline color_t to_color(T r, T g, T b)
	{return color_t(uchar(r)) | color_t(uchar(g) << 8) | color_t(uchar(b) << 16);}

inline color_t rrggbb_to_color(long v) {
	uchar r = uchar((v & 0xFF0000) >> 16);
	uchar g = uchar((v & 0xFF00) >> 8);
	uchar b = uchar(v & 0xFF);
	return to_color(r, g, b);
}

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

	template <class T>
	color_trible(T _r, T _g, T _b) : b(uchar(_b)), g(uchar(_g)), r(uchar(_r)) {}

	color_trible(color_t rgb) : b(bluec(rgb)), g(greenc(rgb)), r(redc(rgb)) {}

	color_trible(const color_quad& q) : b(q.b), g(q.g), r(q.r) {}

	uchar blue() const { return b;}
	uchar green() const { return g;}
	uchar red() const { return r;}

	template <class T>
	void blue(T _b) { b = uchar(_b);}

	template <class T>
	void green(T _g) { g = uchar(_g);}

	template <class T>
	void red(T _r) { r = uchar(_r);}

	color_trible& operator=(color_t c) {
		b = bluec(c);
		g = greenc(c);
		r = redc(c);
		return *this;
	}

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
	static int get_bits_size() { return 24;}
};

// compute chroma_low, chroma_high
void compute_chroma_range(
	lib::color_t chroma_key,
	lib::color_t chroma_keytolerance,
	lib::color_t* p_chroma_low,
	lib::color_t* p_chroma_high);

// test if given color 'c' is between' c_low' and 'c_high'
bool color_t_in_range(lib::color_t c, lib::color_t c_low, lib::color_t c_high);

} // namespace lib

} // namespace ambulant


#include <ostream>

inline std::ostream& operator<<(std::ostream& os, const ambulant::lib::color_trible& t) {
	return os << '(' << int(t.r) << ", " << int(t.g) << ", " << int(t.b)  << ')';
}

#endif // AMBULANT_LIB_COLORS_H
