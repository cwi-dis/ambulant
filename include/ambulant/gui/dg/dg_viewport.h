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
	void redraw(const lib::screen_rect<int>& rc);
	void redraw(HDC hdc);
	
	// Clears the back buffer using this viewport bgd color
	void clear();

	// Clears the specified back buffer rectangle using the provided color 
	void clear(const lib::screen_rect<int>& rc, lib::color_t clr);

	// Draw the whole DD surface to the back buffer and destination rectangle
	void draw(dib_surface_t* src, const lib::screen_rect<int>& dst_rc, bool keysrc = false);
	
	// Draw the src_rc of the DD surface to the back buffer and destination rectangle
	void draw(dib_surface_t* src, const lib::screen_rect<int>& src_rc,
		const lib::screen_rect<int>& dst_rc, bool keysrc = false);
	
	// Draw the text to the back buffer within destination rectangle
	void draw(const std::basic_string<text_char>& text, const lib::screen_rect<int>& dst_rc, lib::color_t clr = 0);
	
	// Draw a frame around the provided rect
	void frame_rect(const lib::screen_rect<int>& rc, lib::color_t clr = 0xFF0000);
	
	int get_width() const { return m_width;}
	
	int get_height() const { return m_height;}
	
	HWND get_hwnd() { return m_hwnd;}
	
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
