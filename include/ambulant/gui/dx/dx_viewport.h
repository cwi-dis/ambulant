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
class dx_transition;

// A viewport is a top-level DD surface.

// white is used as transparent color
const lib::color_t CLR_DEFAULT		= RGB(255, 255, 255);
// almost white is used as alternative color for white
const lib::color_t CLR_ALTERNATIVE	= RGB(255, 255, 254);

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
	
	// Surfaces cashe
	IDirectDrawSurface* create_surface();
	void release_surface(IDirectDrawSurface* surf);
	
	// Blt back buffer to primary surface
	void redraw();
	void redraw(const lib::rect& rc);
	void redraw(HDC hdc) { redraw();}
	void schedule_redraw();
	void schedule_redraw(const lib::rect& rc);
	
	// Clears the back buffer using this viewport bgd color
	void clear();

	// Clears the specified back buffer rectangle using the provided color 
	// and taking into account any transition
	void clear(const lib::rect& rc, lib::color_t clr, double opacity, dx_transition *tr = 0);
	
	// Clears the specified buffer rectangle using the provided color 
	void clear(const lib::rect& rc, lib::color_t clr, double opacity, IDirectDrawSurface* dstview);
	
	// Clears a DD surface with the provided color.
	void clear_surface(IDirectDrawSurface* p, lib::color_t clr, double opacity);

	// Draw the whole DD surface to the back buffer and destination rectangle
	void draw(IDirectDrawSurface* src, const lib::rect& dst_rc, bool keysrc = false);
	
	// Draw the src_rc of the DD surface to the back buffer and destination rectangle
	void draw(IDirectDrawSurface* src, const lib::rect& src_rc,
		const lib::rect& dst_rc, bool keysrc = false, dx_transition *tr = 0);
	
	// Draw the src_rc of the DD surface to the dstview buffer and destination rectangle
	void draw(IDirectDrawSurface* src, const lib::rect& src_rc,
		const lib::rect& dst_rc, bool keysrc, IDirectDrawSurface* dstview);
	

	// Draw the text to the back buffer within destination rectangle
	void draw(const std::basic_string<text_char>& text, const lib::rect& dst_rc, lib::color_t clr = 0);
	
	// Blits (src, src_rc) to (dst, dst_rc)
	void blit(IDirectDrawSurface* src, const lib::rect& src_rc,
		IDirectDrawSurface* dst, const lib::rect& dst_rc);

	// Copies to the DD surface the bits of the back buffer within the from rect
	void rdraw(IDirectDrawSurface* dst, const lib::rect& from_rc);
	
	// Creates a copy of the bgd 
	void copy_bgd_to(IDirectDrawSurface* surf, const lib::rect& rc);
	
	// Draw the copy using the clipping region
	void draw_to_bgd(IDirectDrawSurface* surf, const lib::rect& rc, HRGN hrgn);
	
	// Fading, opacity and chromakeying support
	//
	// blend each pixel in 'from' with the corresponding one in 'to' using 'opacity'
	// when the color of the pixel in 'from' is in the range [chroma_low, chroma_high]
	// otherwise the pixel is copied.
	bool blt_blend (IDirectDrawSurface* to, IDirectDrawSurface* from,
		const lib::rect& rc, double opacity,
		lib::color_t chroma_low, lib::color_t chroma_high, bool copy);
	// blend 'src' surface into m_surface using specified opacity within 
	// [chroma_low, chroma_high]; colors outside this interval are copied/ignored.
	void blend_surface(const lib::rect& dst_rc, IDirectDrawSurface* src,
		const lib::rect& src_rc, bool keysrc, double opacity,
		lib::color_t chroma_low, lib::color_t chroma_high, bool copy);
	// blend 'src' surface into 'dst' surface using specified opacity within
	// [chroma_low, chroma_high]; colors outside this interval are copied/ignored.
	void blend_surface(IDirectDrawSurface* dst, const lib::rect& dst_rc, 
		IDirectDrawSurface* src, const lib::rect& src_rc, bool keysrc, double opacity,
		lib::color_t chroma_low, lib::color_t chroma_high, bool copy);
	// copy 'src_rc' rectangle in 'src' surface into 'dst' surface' 'dst_rc' rectangle
	void copy_surface(IDirectDrawSurface* dst, const lib::rect& dst_rc, IDirectDrawSurface* src, const lib::rect& src_rc);

		// Paints the provided rect
	void frame_rect(const lib::rect& rc, lib::color_t clr = 0xFF0000);
	
	// Helper, that returns the size of a DD surface 
	static lib::size get_size(IDirectDrawSurface *p);
	
	// Returns the DD object associated with this
	IDirectDraw* get_direct_draw() { return m_direct_draw;}
	
	// Returns the DD surface that this view uses as backbuffer
	IDirectDrawSurface* get_surface() { return m_surface; }

	// Converts lib::color_t to a DD color value.
	uint32 convert(lib::color_t color);
	uint32 convert(BYTE r, BYTE g, BYTE b);
	
	int get_width() const { return m_width;}
	int get_height() const { return m_height;}
	HWND get_hwnd() { return m_hwnd;}
	
	void set_fullscreen_transition(dx_transition *tr) { m_fstransition = tr; }
  private:
	
	RECT* to_screen_rc_ptr(RECT& r);
	void get_pixel_format();
	uint16 low_bit_pos(uint32 dword);
	uint16 high_bit_pos(uint32 dword);

	void get_low_high_values(uint32 low_ddclr, uint32 high_ddclr, BYTE* r_l, BYTE* r_h, BYTE* g_l, BYTE* g_h, BYTE* b_l, BYTE* b_h);
	HRESULT blt_blend32(const lib::rect& rc, double progress,
		IDirectDrawSurface *surf1, IDirectDrawSurface *surf2,
		uint32 low_ddclr, uint32 high_ddclr, bool copy);
	HRESULT blt_blend24(const lib::rect& rc, double progress,
		IDirectDrawSurface *surf1, IDirectDrawSurface *surf2,
		uint32 low_ddclr, uint32 high_ddclr, bool copy);
	HRESULT blt_blend16(const lib::rect& rc, double progress,
		IDirectDrawSurface *surf1, IDirectDrawSurface *surf2, uint32 low_ddclr,
		uint32 high_ddclr, bool copy);

	HWND m_hwnd;
	IDirectDraw* m_direct_draw;
	IDirectDrawSurface* m_primary_surface;
	IDirectDrawSurface* m_surface;
	IDirectDrawSurface* m_fstr_surface;
	dx_transition* m_fstransition;
	std::list<IDirectDrawSurface*> m_surfaces;
	
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

inline void set_rect(const lib::rect& rc, RECT *r) {
	r->left = rc.left();
	r->top = rc.top();
	r->right = rc.right();
	r->bottom = rc.bottom();
}

inline bool is_empty_region(HRGN hrgn) {
	RECT rc;
	return (GetRgnBox(hrgn, &rc) == NULLREGION) || rc.top == rc.bottom || rc.left == rc.right;
}

} // namespace dx

} // namespace gui

} // namespace ambulant 

#endif // AMBULANT_GUI_DX_VIEWPORT_H
