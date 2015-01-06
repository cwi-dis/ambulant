// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/lib/colors.h"

#include <string>
#include <map>

namespace ambulant {

namespace lib {

typedef std::map<std::string, color_t> colors_map;

static colors_map html4_colors_map;

bool is_color(const char *name) {
	if (name[0] == '#') {
		char *endp;
		long int val = strtol(name+1, &endp, 16);
		return (*endp == '\0' && val != LONG_MIN && val != LONG_MAX);
	}
	colors_map::const_iterator it = html4_colors_map.find(name);
	return !(it == html4_colors_map.end());
}


color_t to_color(const char *name) {
	if (name[0] == '#') {
		char *endp;
		long hexcolor = strtol(name+1, &endp, 16);
		if (*endp != '\0') return 0;
		return rrggbb_to_color(hexcolor);
	}
	colors_map::const_iterator it = html4_colors_map.find(name);
	if(it == html4_colors_map.end()) return 0;
	return (*it).second;
}

struct colors_init {
	colors_init() {
		// HTML 4.0 spec color values
		entry("aqua", 0x00, 0xFF, 0xFF);
		entry("black", 0x00, 0x00, 0x00);
		entry("blue", 0x00, 0x00, 0xFF);
		entry("fuchsia", 0xFF, 0x00, 0xFF);
		entry("gray", 0x80, 0x80, 0x80);
		entry("green", 0x00, 0x80, 0x00);
		entry("lime", 0x00, 0xFF, 0x00);
		entry("maroon", 0x80, 0x00, 0x00);
		entry("navy", 0x00, 0x00, 0x80);
		entry("olive", 0x80, 0x80, 0x00);
		entry("purple", 0x80, 0x00, 0x80);
		entry("red", 0xFF, 0x00, 0x00);
		entry("silver", 0xC0, 0xC0, 0xC0);
		entry("teal", 0x00, 0x80, 0x80);
		entry("white", 0xFF, 0xFF, 0xFF);
		entry("yellow", 0xFF, 0xFF, 0x00);
		entry("orange", 0xFF, 0xA5, 0x00);
	}
	void entry(const char *name, uchar r, uchar g, uchar b){
		html4_colors_map[name] = to_color(r, g, b);
	}
} colors_init_inst;


// compute chroma_low, chroma_high
void
compute_chroma_range(
	lib::color_t chromakey,
	lib::color_t chromakeytolerance,
	lib::color_t* p_chroma_low,
	lib::color_t* p_chroma_high)
{
	lib::color_t chroma_low, chroma_high;
	if ((int)chromakeytolerance == 0) {
		chroma_low = chroma_high = chromakey;
	} else {
		uchar rk = redc(chromakey),
			gk = greenc(chromakey),
			bk = bluec(chromakey);
		uchar rt = redc(chromakeytolerance),
			gt = greenc(chromakeytolerance),
			bt = bluec(chromakeytolerance);
		uchar rl=0, rh=255, gl=0, gh=255, bl=0, bh=255;
		if (rk - rt > 0)   rl = rk - rt;
		if (rk + rt < 255) rh = rk + rt;
		if (gk - gt > 0)   gl = gk - gt;
		if (gk + gt < 255) gh = gk + gt;
		if (bk - bt > 0)   bl = bk - bt;
		if (bk + bt < 255) bh = bk + bt;
		chroma_low	= to_color(rl, gl, bl);
		chroma_high = to_color(rh, gh, bh);
	}
	if (p_chroma_low) *p_chroma_low = chroma_low;
	if (p_chroma_high) *p_chroma_high = chroma_high;
}

// test if given color 'c' is between' c_low' and 'c_high'
bool
color_t_in_range(lib::color_t c, lib::color_t c_low, lib::color_t c_high)
{
	uchar r_c = redc(c), r_l = redc(c_low), r_h = redc(c_high);
	uchar g_c = greenc(c), g_l = greenc(c_low), g_h = greenc(c_high);
	uchar b_c = bluec(c), b_l = bluec(c_low), b_h = bluec(c_high);
	if ( // check all components in color range
		r_l <= r_c && r_c <= r_h
		&&	g_l <= g_c && g_c <= g_h
		&&	b_l <= b_c && b_c <= b_h)
	{
		return true;
	} else {
		return false;
	}
}

} // namespace lib

} // namespace ambulant
