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

#ifndef AMBULANT_GUI_SDL_WINDOW_H
#define AMBULANT_GUI_SDL_WINDOW_H

// TBD: fullscreen transitions, screenshots

#include "ambulant/common/factory.h"
#include "ambulant/common/recorder.h"
#include "ambulant/common/gui_player.h"
#include "ambulant/common/playable.h"
#include "ambulant/lib/colors.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/smil2/transition.h"

#include "SDL.h"
#include <stack>

namespace ambulant {
namespace gui {
namespace sdl {

/// SDL implementation of gui_window

/// ambulant_sdl_window is the SDL implementation of gui_window, it is the
/// class that corresponds to a SMIL topLayout element.
/// It interfaces with sdl_ambulant_window.
/// Modeled after cg_gui.

inline SDL_Rect SDL_Rect_from_ambulant_rect (const lib::rect& r) {
	SDL_Rect rv = {r.left(), r.top(), r.width(), r.height()};
	return rv;
}
inline ambulant::lib::rect ambulant_rect_from_SDL_Rect (const SDL_Rect& r) {
	return lib::rect(lib::point(r.x, r.y), lib::size(r.w, r.h));
}
inline SDL_Color SDL_Color_from_ambulant_color(ambulant::lib::color_t ac) {
  SDL_Color sc = {ambulant::lib::redc(ac), ambulant::lib::greenc(ac), ambulant::lib::bluec(ac), 255};
	return sc;
}
inline ambulant::lib::color_t ambulant_color_from_SDL_Color (SDL_Color sc) {
	ambulant::lib::color_t ac = ambulant::lib::to_color (sc.r*sc.a/255, sc.g*sc.a/255, sc.b*sc.a/255);
	return ac;
}

class sdl_ambulant_window;

class ambulant_sdl_window : public common::gui_window {
  public:
	ambulant_sdl_window(const std::string &name, lib::rect* bounds, common::gui_events *region);
	~ambulant_sdl_window();

	// gui_window API:
	void need_redraw(const lib::rect &r);
	void redraw(const lib::rect &r);
	void redraw_now();

	// gui_events API:
	bool user_event(const lib::point &where, int what=0);
	void need_events(bool want);

	// semi-private helpers:

	/// Set the corresponding window.
	void set_ambulant_window(sdl_ambulant_window* sdlaw);

	/// Get the sdl_ambulant_window corresponding to this ambulant window.
	sdl_ambulant_window* get_sdl_ambulant_window();

	/// Set our top-level gui_player.
	void set_gui_player(common::gui_player* gpl);

	/// Get our top-level gui_player.
	common::gui_player* get_gui_player();

	lib::rect get_bounds() { return m_bounds; }

	// resize window to new size
	void resize_window (int w, int h);

//TBD	These need to be documented...
//TBD	SDL_Surface* get_ambulant_surface(); //XX
//TBD	SDL_Surface* new_ambulant_surface(); //XX
//TBD	SDL_Surface* get_ambulant_window(); //XX
//TBD	SDL_Surface* get_ambulant_oldsurface(); //XX
//TBD	SDL_Surface* get_surface_from_screen(const lib::rect &r); //XX
//TBD	void reset_ambulant_surface(void); //XX
//TBD	void set_ambulant_surface(SDL_Surface* surf); //XX
//TBD	void delete_ambulant_surface();
  private:
	lib::rect  m_bounds;
	sdl_ambulant_window* m_ambulant_window;
//TBD	uint8_t* m_pixels;
//TBD	SDL_Surface* m_surface;
//TBD	SDL_Surface* m_oldsurface;
//TBD	SDL_Surface* m_fullscreen_prev_surface;
//TBD	SDL_Surface* m_fullscreen_old_surface;
//TBD	smil2::transition_engine* m_fullscreen_engine;
//TBD	lib::transition_info::time_type m_fullscreen_now;
	/// A renderer is used for drawing, contains all drawing attribute (like a grapohics context)
//TBD	SDL_Renderer* m_sdl_renderer;
	/// A surface contains the actual pixels of the window
//TBD	SDL_Surface*  m_sdl_surface;
	/// When the 'ambulant_recorder_plugin' (in sandbox) is installed, this renderer will feed it with screen grabs
	common::gui_player* m_gui_player;
	common::recorder* m_recorder;
	lib::critical_section m_lock;

  public:
	SDL_Surface* m_tmpsurface;
};  // class ambulant_sdl_window

/// sdl_ambulant_window is the SDL-counterpart of ambulant_sdl_window: it is the
/// SDL_Window that corresponds to an Ambulant topLayout window.
/// It interfaces with SDL2, used for communicating (mouse) events from ambulant
/// and creating/accessing SDL functions by ambulant
class sdl_ambulant_window : public ambulant::common::gui_screen
{
  public:
	sdl_ambulant_window(SDL_Window* window);
	~sdl_ambulant_window();

	/// Helper: get the actual SDL_Window
	SDL_Window* get_sdl_window() { return m_sdl_window; }

	/// Helper: get the actual SDL_Renderer for the surface
	SDL_Renderer* get_sdl_renderer() { return m_sdl_renderer; }

	/// Helper: get the actual SDL_Renderer for the window
	SDL_Renderer* get_sdl_window_renderer() { return m_sdl_window_renderer; }

	/// Helper: set the drawing SDL_Surface
	void set_sdl_surface(SDL_Surface* s);

	/// Helper: get the current SDL_Surface used for drawing
	SDL_Surface* get_sdl_surface() { return m_sdl_surface; }

	/// Helper: delete the drawing SDL_Surface
	void delete_sdl_surface();

	/// Helper: get the SDL_Surface used during transitions, create if needed
	SDL_Surface* get_transition_surface();

	/// Helper: delet the SDL_Surface used during transitions
	void delete_transition_surface();

	/// Helper: get the actual SDL_Surface of the screen 
//	SDL_Surface* get_sdl_screen_surface() { return m_sdl_screen_surface; }

	/// Helper: set our counterpart gui_window.
	void set_ambulant_sdl_window( ambulant_sdl_window* asdlw);

	/// Helper: get our counterpart gui_window.
	ambulant_sdl_window* get_ambulant_sdl_window() { return m_ambulant_sdl_window; } 

	/// Helper: copy and scale the surface 'src' to the current surface (using a blit operation)
	int copy_to_sdl_surface (SDL_Surface* src, SDL_Rect* src_rect, SDL_Rect* dst_rect, Uint8 alpha, SDL_Rect* clip_rect = NULL);
	/// Helper: copy the surface 'src' to the current surface (using a scaled blit operation)
	/// s.t. the area 'src_rect' from the SDL_Surface 'src' fits in the area 'dst_rect' of 'dst'
	/// This function can be used when both 'src_rect' and 'dst_rect' are properly computed by
	/// 'get_fit_rect' to implement the SMIL 3.0 'fit' semanics
//TBD	int copy_to_sdl_screen_surface (SDL_Surface* src, SDL_Rect* src_rect, SDL_Rect* dst_rect, Uint8 alpha); // do i need this one ?
	bool get_sdl_fullscreen () { return m_sdl_fullscreen; }
	/// Debug aids
	void dump_sdl_surface (SDL_Surface* surf, const char* id);
	void dump_sdl_renderer (SDL_Renderer* renderer, SDL_Rect rect, const char* id);

	void set_evp (lib::event_processor* evp) { m_evp = evp; }
	lib::event_processor* get_evp() { return m_evp; }

	// gui_screen implementation
	void get_size(int *width, int *height) {} //TBD
	bool get_screenshot(const char *type, char **out_data, size_t *out_size) { return false; } //TBD

	SDL_PixelFormat* get_window_pixel_format () { return m_window_pixel_format; }

	/// Transitions

	void startScreenTransition();
	void endScreenTransition();
	void screenTransitionStep(smil2::transition_engine* engine, lib::transition_info::time_type now);

	void _screenTransitionPreRedraw();
	void _screenTransitionPostRedraw(const lib::rect &r);
	/// SDL_Surface handling

	/// Copy surface and pixels 
	SDL_Surface* copy_sdl_surface(SDL_Surface* surface);
	/// Push a SDL_Surface on the transition surface stack
	void push_sdl_surface(SDL_Surface* s);
	/// Get the topmost SDL_Surface from the transition surface stack
	SDL_Surface* top_sdl_surface (void) { 
		return m_transition_surfaces.empty() ? NULL : m_transition_surfaces.top();
	}
	/// Pop the topmost SDL_Surface from the transition surface stack and return it
	SDL_Surface* pop_sdl_surface (void) { 
		SDL_Surface* s = top_sdl_surface();
		if ( ! m_transition_surfaces.empty()) m_transition_surfaces.pop();
		return s;
	}
	/// Clear a rectangle of the pixels in the SDL_Surface and prepare for drawing after resize window
	/// Returns true if another redraw mst be scheduled
	bool clear_sdl_surface (lib::rect);

	/// return the corresponding sdl_ambulant_window* given its SDL windowID (used by SDL event loop)
	static sdl_ambulant_window* get_sdl_ambulant_window  (Uint32 windowID);
//TBD ?	bool set_screenshot(char **screenshot_data, size_t *screenshot_size);
	// For the gui_screen implementation
	void* m_screenshot_data;
	long int m_screenshot_size;
	lib::point transform (SDL_Point p); 
	SDL_Rect get_sdl_dst_rect() { return m_sdl_dst_rect; } //XX should be private
	// resize window to new size
	void sdl_resize_window (int w, int h);
	void need_redraw(ambulant_sdl_window* asw, lib::rect r);
	void redraw(lib::rect r);
	const char* get_screen_pixels();
	void remove_redraw_SDL_Events(ambulant_sdl_window* asw);
  private:
	// Helper: create the actual SDL_Window*, foreground and background pixels, surfaces and renderers
	int create_sdl_window_and_renderers(const char* window_name, lib::rect);
	int create_sdl_surface_and_pixels(SDL_Rect*, uint8_t** pixels=NULL, SDL_Surface** surface=NULL, SDL_Renderer** renderer=NULL);
	SDL_Rect compute_sdl_dst_rect(int w, int h, SDL_Rect r);	
	ambulant_sdl_window* m_ambulant_sdl_window;
	// The actual SDL_Window*
	SDL_Window*   m_sdl_window;
	SDL_Renderer* m_sdl_window_renderer;
	// A surface contains the current surface for drawing
	SDL_Surface*  m_sdl_surface;
	SDL_Surface*  m_sdl_transition_surface;
	SDL_Renderer* m_sdl_renderer;
	// The screen_surface/renderer represent the actual pixels of the window
	SDL_Surface*  m_sdl_screen_surface;
	SDL_Renderer* m_sdl_screen_renderer; // the "real" renderer, for SDL_Present()
	bool m_sdl_fullscreen;
	SDL_Rect m_document_rect;
	SDL_Rect m_sdl_dst_rect;
	int m_sdl_window_flags;
	float m_sdl_scale;
	lib::event_processor* m_evp;
	uint8_t* m_screen_pixels;
	// window counter (with s_lock protection) is used to assuere that the SdlWindow
	// in drawing callback functions are still valid pointers at the time the callback
	// is executed by the main thread */
	static lib::critical_section s_lock;
	static int s_windows;
	// sdl_ambulant_window maintains 2 mappings:
	// - s_window_renderer_map: to find a SDL_Renderer* given a SDL_Window* (for drawing) 
	// - s_id_sdl_ambulant_window_map: to find a sdl_ambulant_window* given a window_id
	static std::map<int, sdl_ambulant_window*> s_id_sdl_ambulant_window_map;

	std::stack<SDL_Surface*> m_transition_surfaces;
	int m_fullscreen_count;
	bool m_need_window_resize;
	int m_new_width;
	int m_new_height;
	
	SDL_PixelFormat* m_window_pixel_format;

// The total number of SDL events at any moment is maintained in order to clear
// the SDL Event Queue of pointers to this structure upon deletion
	static long unsigned int s_num_events;

};  // class sdl_ambulant_window
// TMP: Experimental optimization: use the screen SDL_PixelFormat in renderers
#define WITH_DYNAMIC_PIXEL_LAYOUT
// TMP: Experimental optimization: turn on WITH_SDL_TEXTURE to update
// the screen using SDL_Renderer and SDL_texture
// this is the way to work with SDL-2.x since when properly used more work can
// offloaded from CPUs to graphics cards.
// We currently turn this off and use SDL-2.x in a way equivalent SDL-1.x
// because it seems SDL than select faster blit functions, apparently because
// we don't have learned yet how to use SDL-2.x properly 
//#define WITH_SDL_TEXTURE


} // namespace sdl

} // namespace gui

} // namespace ambulant

#endif//AMBULANT_GUI_SDL_WINDOW_H

