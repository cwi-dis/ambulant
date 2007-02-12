// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

/* 
 * @$Id$ 
 */

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
		(void)strtol(name+1, &endp, 16);
		return (*endp == '\0');
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

} // namespace lib
 
} // namespace ambulant
