/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2007 Stichting CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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

/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_DG_VIEWPORT_H
#define AMBULANT_GUI_DG_VIEWPORT_H

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

#include "ambulant/gui/dg/dg_surface.h"
#include "ambulant/gui/dg/dg_dib_surface.h"

namespace ambulant {
namespace lib {
	class logger; 
}}


namespace ambulant {

namespace gui {

namespace dg {

//using lib::uint16;
//using lib::uint32;
//using lib::uchar;

// A viewport is a top-level DG surface.

class viewport {
  public:
	viewport(int width, int height, HWND hwnd);
	~viewport();
	
	// Sets the background color of this viewport
	void set_background(lib::color_t color);
		
	// Blt back buffer to primary surface
	void redraw();
	void redraw(const lib::rect& rc);
	void redraw(HDC hdc);
	
	// Clears the back buffer using this viewport bgd color
	void clear();

	// Clears the specified back buffer rectangle using the provided color 
	void clear(const lib::rect& rc, lib::color_t clr);

	// Draw the whole DD surface to the back buffer and destination rectangle
	void draw(dib_surface_t* src, const lib::rect& dst_rc,  
		bool keysrc = false, lib::color_t transp = CLR_INVALID);
	
	// Draw the src_rc of the DD surface to the back buffer and destination rectangle
	void draw(dib_surface_t* src, const lib::rect& src_rc,
		const lib::rect& dst_rc, bool keysrc = false, lib::color_t transp = CLR_INVALID);
	
	// Draw the text to the back buffer within destination rectangle
	void draw(const std::basic_string<text_char>& text, const lib::rect& dst_rc, 
		lib::color_t clr = CLR_INVALID, const char *fontname = NULL,
		float fontsize = 0);
	
	// Draw a frame around the provided rect
	void frame_rect(const lib::rect& rc, lib::color_t clr = 0xFF0000);
	
	int get_width() const { return m_width;}
	
	int get_height() const { return m_height;}
	
	HWND get_hwnd() { return m_hwnd;}
	
	// Create a new offscreen bitmap with the given size. The bitmap is
	// owned by the dib_surface_t object returned.
	dib_surface_t* create_surface(DWORD w, DWORD h);

	// Return the bitmap containing the current screen bits. The surface_t
	// is not a copy, it is a borrowed reference.
	surface_t* get_surface();

  private:
	int m_width;
	int m_height;
	HWND m_hwnd;
	surface_t *m_surf;
	HDC m_memdc;
	HBITMAP m_hbmp;
	HBITMAP m_hold;
	lib::color_t m_bgd;
	int m_bits_size; // fixed to 24 for dg
	lib::logger *m_logger;
};

} // namespace dg

} // namespace gui

} // namespace ambulant 

#endif // AMBULANT_GUI_DG_VIEWPORT_H
