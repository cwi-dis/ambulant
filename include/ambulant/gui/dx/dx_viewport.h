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

#ifndef AMBULANT_LIB_DX_VIEWPORT_H
#define AMBULANT_LIB_DX_VIEWPORT_H

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

#include "ambulant/config/config.h"

#include <string>
#include <list>
#include "ambulant/lib/colors.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/mtsync.h"
#include "ambulant/lib/layout.h"
#include "ambulant/common/region.h"

#include "ambulant/common/preferences.h"

struct IDirectDraw;
struct IDirectDrawSurface;
struct tagPALETTEENTRY;
namespace ambulant {

namespace gui {

namespace dx {

using namespace ambulant;
using lib::uint16;
using lib::uint32;
using lib::uchar;

class region;
class video_player;

class viewport {
  public:
	viewport(int width = lib::default_layout_width, 
		int height = lib::default_layout_height, HWND hwnd = NULL);
	~viewport();
	
	// Shows what has been drawn
	// Blits back surface to front.
	void redraw();
		
	// Sets the background color (to be used by clear)
	void set_background(lib::color_t color) {
		m_bgd = color;
	}
	template <class T>
	void set_background(T r, T g, T b) {
		m_bgd = lib::to_color(r, g, b);
	}
	void set_background(const char *name) {
		m_bgd = lib::to_color(name);
	}
	
	// fills the drawing surface 
	// with the current background color
	void clear();	
	
	IDirectDrawSurface* create_surface(DWORD w, DWORD h, DWORD ddcolor = CLR_INVALID);
	
	// Creates a new region.
	// rc: The coordinates of the region relative to this viewport
	// crc: The coordinates of the parent or cliping region relative to this viewport
	region* create_region(const lib::screen_rect<int>& rc, const lib::screen_rect<int>& crc, int zindex);
	void remove_region(region *r);
	
	// ddraw services
	uint32 convert(lib::color_t color);
	uint32 convert(uchar r, uchar g, uchar b);
	
	IDirectDraw* get_direct_draw() { return m_direct_draw;}
	
	lib::screen_rect<int> get_rc() const { 
		return lib::screen_rect<int>(lib::point(0, 0), lib::point(m_width, m_height));}
	lib::screen_rect<int> get_clip_rc() const { 
		return lib::screen_rect<int>(lib::point(0, 0), lib::point(m_width, m_height));}
	int get_width() const { return m_width;}
	int get_height() const { return m_height;}
	
	void set_layout(const lib::passive_region *layout) { m_layout = layout; }
	
  private:	
 	void add_region(region *r);
 
	RECT* to_screen_rc_ptr(RECT& r);
	
	// Draw the region.
	void draw(region *r);
	
	void get_pixel_format();
	uint16 low_bit_pos(uint32 dword);
	uint16 high_bit_pos(uint32 dword);

	// link to the layout
	const lib::passive_region *m_layout;
	
	HWND m_hwnd;
	IDirectDraw* m_direct_draw;
	IDirectDrawSurface* m_primary_surface;
	IDirectDrawSurface* m_surface;
	
	int m_width, m_height;
	lib::color_t m_bgd;
	
	std::list<region*> m_regions;
	lib::critical_section m_regions_cs;  
	
	// surface pixel format
	uint32 bits_size;
	uint16 red_bits, green_bits, blue_bits;
	uint16 lo_red_bit, lo_green_bit, lo_blue_bit;
	tagPALETTEENTRY* palette_entries;
};

class region {
  public:
	region(viewport *v, const lib::screen_rect<int>& rc, 
		const lib::screen_rect<int>& crc, IDirectDrawSurface* surface); 
	~region();
	
	viewport* get_viewport() { return m_viewport;}
	const lib::screen_rect<int>& get_rc() const { return m_rc;}
	const lib::screen_rect<int>& get_clip_rc() const { return m_clip_rc;}
	
	IDirectDrawSurface* get_surface() { return m_surface;}
	
	// Sets the background color (to be used by clear)
	void set_background(lib::color_t color) {
		m_bgd = color;
	}
	template <class T>
	void set_background(T r, T g, T b) {
		m_bgd = lib::to_color(r, g, b);
	}
	void set_background(const char *name) {
		m_bgd = lib::to_color(name);
	}
	
	void clear();
	void update();
	
	void set_text(const char *p, int size);
	void set_text(const std::string& what);
	void set_bmp(HBITMAP hbmp, int width, int height, bool transp, lib::color_t tarnsp_color);
	void set_video(video_player *player);
	
	void set_rendering_surface(const lib::abstract_rendering_surface *rsurf) { m_rsurf = rsurf;}
	void set_rendering_info(const lib::abstract_smil_region_info *rinfo) { m_rinfo = rinfo;}
	
	const lib::abstract_rendering_surface* get_rsurf() const { return m_rsurf;}
	const lib::abstract_smil_region_info* get_rinfo() const { return m_rinfo;}
	
	bool is_transparent() const { 
		return m_rinfo?m_rinfo->get_transparent():false;
	}
	
  private:
	// The viewport of this region.
	viewport *m_viewport;	
	
	// links to the layout
	const lib::abstract_rendering_surface *m_rsurf;
	const lib::abstract_smil_region_info *m_rinfo;
	
	// The coordinates of this region 
	// relative to the viewport
	lib::screen_rect<int> m_rc;
	
	// The coordinates of the parent region 
	// relative to the viewport
	lib::screen_rect<int> m_clip_rc;
	
	// The surface of this region.
	// m_rc.width() x m_rc.height()
	IDirectDrawSurface* m_surface;
	
	// The background color of this region.
	lib::color_t m_bgd;	
	
	// image
	IDirectDrawSurface* m_imgsurf;
	
	// video
	video_player *m_video_p;
	
};

} // namespace dx

} // namespace gui

} // namespace ambulant 

#endif // AMBULANT_LIB_DX_VIEWPORT_H
