/* 
 * @$Id$ 
 */

#ifndef AMBULANT_LIB_DX_VIEWPORT_H
#define AMBULANT_LIB_DX_VIEWPORT_H

#include <string>
#include <list>
#include "ambulant/lib/colors.h"
#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/mtsync.h"

struct IDirectDraw;
struct IDirectDrawSurface;
struct tagPALETTEENTRY;

#include <windows.h>

namespace ambulant {

namespace gui {

namespace dx {

using namespace ambulant;
using lib::uint16;
using lib::uint32;
using lib::uchar;

class region;

class viewport {
  public:
	viewport(int width = 320, int height = 240, HWND hwnd = NULL);
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
	
	// Creates a new region.
	// rc: The coordinates of the region relative to this viewport
	// crc: The coordinates of the parent or cliping region relative to this viewport
	region* create_region(const lib::screen_rect<int>& rc, const lib::screen_rect<int>& crc);
	void remove_region(region *r);
	
	// ddraw services
	uint32 convert(lib::color_t color);
	uint32 convert(uchar r, uchar g, uchar b);
	
  private:	
 	void add_region(region *r);
 
	RECT* to_screen_rc_ptr(RECT& r);
	
	// Draw the region.
	void draw(region *r);
	
	void get_pixel_format();
	uint16 low_bit_pos(uint32 dword);
	uint16 high_bit_pos(uint32 dword);

	
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
	
	void set_text(const char *p, int size);
	void set_text(const std::string& what);
	void set_bmp(HBITMAP hbmp);
	
  private:
	// The viewport of this region.
	viewport *m_viewport;	
	
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
};

} // namespace dx

} // namespace gui

} // namespace ambulant 

#endif // AMBULANT_LIB_DX_VIEWPORT_H
