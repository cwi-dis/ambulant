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

#ifndef AMBULANT_GUI_DX_VIEWPORT_H
#define AMBULANT_GUI_DX_VIEWPORT_H

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

#include "ambulant/config/config.h"

#include <string>
#include <list>

#include "ambulant/lib/colors.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/common/preferences.h"

struct IDirectDraw;
struct IDirectDrawSurface;
struct tagPALETTEENTRY;

namespace ambulant {

namespace gui {

namespace dx {

using lib::uint16;
using lib::uint32;
using lib::uchar;

// A viewport is a top-level DD surface.

class viewport {
  public:
	viewport(int width = common::default_layout_width, 
		int height = common::default_layout_height, HWND hwnd = NULL);
	~viewport();
	
	// Sets the background color of this viewport
	void set_background(lib::color_t color);
	
	// Creates a DD surface with the provided size.
	// The surface is cleared using the specified color
	IDirectDrawSurface* create_surface(DWORD w, DWORD h);
	IDirectDrawSurface* create_surface(lib::size s) {
		return create_surface(s.w, s.h);
	}
	
	// Blt back buffer to primary surface
	void redraw();
	void redraw(const lib::screen_rect<int>& rc);
	void redraw(RECT *prc);
	
	// Clears the back buffer using this viewport bgd color
	void clear();

	// Clears the specified back buffer rectangle using the provided color 
	void clear(const lib::screen_rect<int>& rc, lib::color_t clr);

	// Clears a DD surface with the provided color.
	void clear_surface(IDirectDrawSurface* p, lib::color_t clr);

	// Draw the whole DD surface to the back buffer and destination rectangle
	void draw(IDirectDrawSurface* src, const lib::screen_rect<int>& dst_rc, bool keysrc = false);
	
	// Draw the src_rc of the DD surface to the back buffer and destination rectangle
	void draw(IDirectDrawSurface* src, const lib::screen_rect<int>& src_rc,
		const lib::screen_rect<int>& dst_rc, bool keysrc = false);
	

	// Draw the text to the back buffer within destination rectangle
	void draw(const std::string& text, const lib::screen_rect<int>& dst_rc, lib::color_t clr = 0);
	
	// Helper, that returns the size of a DD surface 
	static lib::size get_size(IDirectDrawSurface *p);
	
	// Returns the DD object associated with this
	IDirectDraw* get_direct_draw() { return m_direct_draw;}
	
	// Converts lib::color_t to a DD color value.
	uint32 convert(lib::color_t color);
	uint32 convert(BYTE r, BYTE g, BYTE b);
	
	int get_width() const { return m_width;}
	int get_height() const { return m_height;}
	HWND get_hwnd() { return m_hwnd;}
	
  private:
	
	RECT* to_screen_rc_ptr(RECT& r);
	void get_pixel_format();
	uint16 low_bit_pos(uint32 dword);
	uint16 high_bit_pos(uint32 dword);

	HWND m_hwnd;
	IDirectDraw* m_direct_draw;
	IDirectDrawSurface* m_primary_surface;
	IDirectDrawSurface* m_surface;
	
	int m_width, m_height;
	lib::color_t m_bgd;
	uint32 m_ddbgd;
	
	// surface pixel format
	uint32 bits_size;
	uint16 red_bits, green_bits, blue_bits;
	uint16 lo_red_bit, lo_green_bit, lo_blue_bit;
	tagPALETTEENTRY* palette_entries;
};

inline void set_rect(IDirectDrawSurface *p, RECT *r) {
	lib::size s = viewport::get_size(p);
	r->left = r->top = 0;
	r->right = s.w;
	r->bottom = s.h;
}

inline void set_rect(const lib::screen_rect<int>& rc, RECT *r) {
	r->left = rc.left();
	r->top = rc.top();
	r->right = rc.right();
	r->bottom = rc.bottom();
}

} // namespace dx

} // namespace gui

} // namespace ambulant 

#endif // AMBULANT_GUI_DX_VIEWPORT_H
