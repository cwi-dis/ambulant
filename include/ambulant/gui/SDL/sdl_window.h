/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2012 Stichting CWI, 
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


#include "ambulant/common/factory.h"
#include "ambulant/common/recorder.h"
#include "ambulant/common/gui_player.h"
#include "ambulant/common/playable.h"
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

	/// Initialize a GDK cached cursortype
//X	void set_gdk_cursor(GdkCursorType, GdkCursor*);

	/// Return any of GDK cached cursortypes
//X	GdkCursor* get_gdk_cursor(GdkCursorType);

	// XXX These need to be documented...
//XX	SDL_Surface* get_ambulant_surface(); //XX
//XX	SDL_Surface* new_ambulant_surface(); //XX
//XX	SDL_Surface* get_ambulant_window(); //XX
//XX	SDL_Surface* get_ambulant_oldsurface(); //XX
//XX	SDL_Surface* get_surface_from_screen(const lib::rect &r); //XX
//XX	void reset_ambulant_surface(void); //XX
//XX	void set_ambulant_surface(SDL_Surface* surf); //XX
//XX	void delete_ambulant_surface();
  private:
	lib::rect  m_bounds;
	sdl_ambulant_window* m_ambulant_window;
//XX	uint8_t* m_pixels;
//XX	SDL_Surface* m_surface;
//XX	SDL_Surface* m_oldsurface;
	common::gui_player* m_gui_player;
//X	GdkCursor* m_arrow_cursor;
//X	GdkCursor* m_hand1_cursor;
//X	GdkCursor* m_hand2_cursor;
//X	SDL_Surface* m_fullscreen_prev_surface;
//X	SDL_Surface* m_fullscreen_old_surface;
//X	smil2::transition_engine* m_fullscreen_engine;
//X	lib::transition_info::time_type m_fullscreen_now;

// The total number of SDL events at any moment is maintained in order to clear
// the SDL Event Queue of pointers to this structure upon deletion
	static long unsigned int s_num_events;
	/// A renderer is used for drawing, contains all drawing attribute (like a grapohics context)
//XX	SDL_Renderer* m_sdl_renderer;
	/// A surface contains the actual pixels of the window
//XX	SDL_Surface*  m_sdl_surface;
	/// When the 'ambulant_recorder_plugin' (in sandbox) is installed, this renderer will feed it with screen grabs
	common::recorder* m_recorder;
	lib::critical_section m_lock;

  public:
	SDL_Surface* m_tmpsurface;
//X	guint signal_redraw_id;
};  // class ambulant_sdl_window

/// sdl_ambulant_window is the SDL-counterpart of ambulant_sdl_window: it is the
/// SDL_Window that corresponds to an Ambulant topLayout window.
/// It interfaces with SDL2, used for communicating (mouse) events from ambulant
/// and creating/accessing SDL functions by ambulant
class sdl_ambulant_window : public ambulant::common::gui_screen
{
  public:
//	sdl_ambulant_window(const std::string &name,
//			   lib::rect* bounds,
//			   SdlWindow* parent_window);
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

	/// Helper: copy the surface 'src' to the current surface (using a blit operation)
	int copy_to_sdl_surface (SDL_Surface* src, SDL_Rect* src_rect, SDL_Rect* dst_rect, Uint8 alpha);
	/// Helper: copy the surface 'src' to the current surface (using a scaled blit operation)
	/// s.t. the area 'src_rect' from the SDL_Surface 'src' fits in the area 'dst_rect' of 'dst'
	/// This function can be used when both 'src_rect' and 'dst_rect' are properly computed by
	/// 'get_fit_rect' to implement the SMIL 3.0 'fit' semanics
	int copy_to_sdl_surface_scaled (SDL_Surface* src, SDL_Rect* src_rect, SDL_Rect* dst_rect, Uint8 alpha);
//TBD	int copy_to_sdl_screen_surface (SDL_Surface* src, SDL_Rect* src_rect, SDL_Rect* dst_rect, Uint8 alpha);
	bool get_sdl_fullscreen () { return m_sdl_fullscreen; }
	float get_sdl_scale () { return m_sdl_scale; }
	/// Debug aids
	void dump_sdl_surface (SDL_Surface* surf, const char* id);
	void dump_sdl_renderer (SDL_Renderer* renderer, SDL_Rect rect, const char* id);

	void set_evp (lib::event_processor* evp) { m_evp = evp; }
	lib::event_processor* get_evp() { return m_evp; }

/// TBD: Event handling
//X	void do_paint_event (GdkEventExpose * event);
//X	void do_motion_notify_event(GdkEventMotion *event);
//X	void do_button_release_event(GdkEventButton *event);
//X	void do_key_release_event(GdkEventKey *event);
//X	void mouseReleaseEvent(QMouseEvent* e);

	// gui_screen implementation
	void get_size(int *width, int *height) {} //TBD
	bool get_screenshot(const char *type, char **out_data, size_t *out_size) { return false; } //TBD

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
	/// Clear the pixels of a SDL_Surface 
	void clear_sdl_surface (SDL_Surface* surface, SDL_Rect sdl_rect);

	/// return the corresponding sdl_ambulant_window* given its SDL windowID (used by SDL event loop)
	static sdl_ambulant_window* get_sdl_ambulant_window  (Uint32 windowID);
//X	bool set_screenshot(char **screenshot_data, size_t *screenshot_size);
	// For the gui_screen implementation
	void* m_screenshot_data;
	long int m_screenshot_size;
	SDL_Rect get_sdl_dst_rect() { return m_sdl_dst_rect; } //XX should be private

  private:
	// Helper: create the actual SDL_Window*, foreground and background pixels, surfaces and renderers
	int create_sdl_window_and_renderers(const char* window_name, lib::rect);
	int create_sdl_surface_and_pixels(SDL_Rect*, uint8_t** pixels=NULL, SDL_Surface** surface=NULL, SDL_Renderer** renderer=NULL);
	
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
	SDL_Rect m_sdl_dst_rect;
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
	//XX static std::map<SDL_Window*, SDL_Renderer*> s_window_renderer_map; //XX is this really needed ????
	static std::map<int, sdl_ambulant_window*> s_id_sdl_ambulant_window_map;

	std::stack<SDL_Surface*> m_transition_surfaces;
	int m_fullscreen_count;

//X	SDL_Window* m_parent_window;
//X	sdl_ambulant_window* m_parent_window;
//X	gulong m_expose_event_handler_id;
//X	gulong m_motion_notify_handler_id;
//X	gulong m_button_release_handler_id;
//X	gulong m_key_release_handler_id;

};  // class sdl_ambulant_window

} // namespace sdl

} // namespace gui

} // namespace ambulant

#endif//AMBULANT_GUI_SDL_WINDOW_H
