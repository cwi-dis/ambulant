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

#ifndef AMBULANT_NO_IOSTREAMS

#ifndef AMBULANT_NO_OSTREAM
#include <ostream>
#else /*AMBULANT_NO_OSTREAM*/
#include <ostream.h>
#endif/*AMBULANT_NO_OSTREAM*/

inline std::ostream& operator<<(std::ostream& os, const ambulant::lib::color_trible& t) { 
	return os << '(' << int(t.r) << ", " << int(t.g) << ", " << int(t.b)  << ')';
}
#endif

#endif // AMBULANT_LIB_COLORS_H
