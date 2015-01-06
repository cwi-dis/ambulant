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

#ifdef  WITH_SDL_IMAGE// work in progress
// TBD: fullscreen transitions, screenshots

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/lib/colors.h"
#include "ambulant/gui/SDL/sdl_factory.h"
#include "ambulant/gui/SDL/sdl_window.h"
//X #include "ambulant/gui/SDL/sdl_image_renderer.h"
//X #include "ambulant/gui/SDL/sdl_smiltext.h"
//X #include "ambulant/gui/SDL/sdl_util.h"
//X #include "ambulant/gui/SDL/sdl_text_renderer.h"
#include "ambulant/gui/SDL/sdl_video.h"

using namespace ambulant;
using namespace gui::sdl;
using namespace common;
using namespace lib;
using namespace net;
//
// ambulant_sdl_window
//
ambulant_sdl_window::ambulant_sdl_window(const std::string &name,
	lib::rect* bounds,
	common::gui_events *region)
:	common::gui_window(region),
	m_bounds(*bounds),
	m_ambulant_window(NULL),
	m_gui_player(NULL),
	m_recorder(NULL)
//TBD	m_pixels(NULL),
//TBD	m_surface(NULL),
//TBD	m_oldsurface(NULL),
//TBD	m_fullscreen_prev_surface(NULL),
//TBD	m_fullscreen_old_surface(NULL),
//TBD	m_fullscreen_engine(NULL),
//TBD	m_fullscreen_now(0),
//TBD	m_sdl_surface(NULL),
//TBD	m_sdl_renderer(NULL),
//TBD	m_fullscreen_count(0),
//TBD	m_oldsurface(NULL),
//TBD	m_tmpsurface(NULL),
//TBD	m_window(NULL)
{
//TBD	m_surface = NULL;
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::ambulant_sdl_window(%p)",(void *)this);
}

ambulant_sdl_window::~ambulant_sdl_window()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::~ambulant_sdl_window(%p): m_ambulant_window=%p",this,m_ambulant_window);
	// Note that we don't destroy the window, only sver the connection.
	// the window itself is destroyed independently.
	if (m_ambulant_window ) {
		m_ambulant_window->remove_redraw_SDL_Events(this);
		m_ambulant_window->set_ambulant_sdl_window(NULL);
		m_ambulant_window = NULL;
	}
//X	if (m_surface != NULL) {
//X		g_object_unref(G_OBJECT(m_surface));
//X		m_surface = NULL;
//X	}
/** m_oldsurface not to be deleted, it points to either m_surface or m_tmpsurface
	if (m_oldsurface != NULL) {
		g_object_unref(G_OBJECT(m_oldsurface));
		m_oldsurface = NULL;
	}
**/
/** m_tmpsurface not to be deleted, it is not a copy anymore
	if (m_tmpsurface != NULL) {
		g_object_unref(G_OBJECT(m_tmpsurface));
		m_tmpsurface = NULL;
	}
**/
	m_lock.leave();
	return;
}

void
ambulant_sdl_window::need_redraw(const lib::rect &r)
{
	m_lock.enter();
	//AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::need_redraw(%p): ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.width(), r.height());
	if (m_ambulant_window == NULL) {
		lib::logger::get_logger()->error("ambulant_sdl_window::need_redraw(%p): m_ambulant_window == NULL !!!", (void*) this);
		m_lock.leave();
		return;
	}
	m_ambulant_window->need_redraw(this, r);
	m_lock.leave();
}

void
ambulant_sdl_window::redraw(const lib::rect &dummy)
{
	m_lock.enter();
	lib::rect r = m_bounds;
//	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::redraw(%p): redraw starts.", this);
	sdl_ambulant_window* saw = get_sdl_ambulant_window();

	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::redraw(%p): ltrb=(%d,%d,%d,%d)",(void *)this, r.left(), r.top(), r.width(), r.height());
//TBD	screenTransitionPreRedraw();
	bool redraw_needed = saw->clear_sdl_surface(r);
	m_lock.leave();
	if (redraw_needed) {
		need_redraw (m_bounds);
		return;
	}
	m_handler->redraw(r, this);
	m_lock.enter();
#ifdef TBD
//XX?	if ( ! isEqualToPrevious(m_surface))
	_screenTransitionPostRedraw(r);
	gdk_surface_bitblt(
		m_ambulant_window->get_sdl_window()->window, r.left(), r.top(),
		m_surface, r.left(), r.top(),
		r.width(), r.height());
#ifdef WITH_SCREENSHOTS
	GError *error = NULL;
	gint width; gint height;

	gdk_drawable_get_size(m_ambulant_window->get_sdl_window()->window, &width, &height);

	GdkPixbuf* pixbuf = gdk_pixbuf_get_from_drawable(
		NULL,
		m_ambulant_window->get_sdl_window()->window,
		0, 0, 0, 0, 0, width, height);
	if (!gdk_pixbuf_save_to_buffer (pixbuf, &buffer, &buffer_size, "jpeg", &error, "quality", "100", NULL)) {
	if (m_ambulant_window->m_screenshot_data) {
		g_free(m_ambulant_window->m_screenshot_data);
		m_ambulant_window->m_screenshot_data = NULL;
		m_ambulant_window->m_screenshot_size = 0;
	}

	if (!gdk_pixbuf_save_to_buffer (
		pixbuf,
		&m_ambulant_window->m_screenshot_data,
		&m_ambulant_window->m_screenshot_size,
		"jpeg", &error, "quality", "100", NULL))
	{
		printf ("error%s", error->message);
		g_error_free (error);
	}
	g_object_unref (G_OBJECT (pixbuf));
#endif //WITH_SCREENSHOTS
	DUMPSURFACE(m_surface, "top");
#endif//TBD
	if (m_recorder && saw->get_evp()) {
		timestamp_t timestamp = saw->get_evp()->get_timer()->elapsed();
		m_recorder->new_video_data(saw->get_screen_pixels(), m_bounds.width()*m_bounds.height()*SDL_BPP, timestamp);
	}
	saw->redraw(r);
	m_lock.leave();
//	lib::logger::get_logger()->debug("ambulant_sdl_window::redraw(%p): redraw done.\n", this);
}

void
ambulant_sdl_window::redraw_now()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::redraw_now()");
}

bool
ambulant_sdl_window::user_event(const lib::point &where, int what)
{
//	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::user_event(%p): point=(%d,%d)", this, where.x, where.y);
	return m_handler->user_event(where, what);
}

void
ambulant_sdl_window::need_events(bool want)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::need_events(%p): want=%d", this, want);
}

void
ambulant_sdl_window::set_ambulant_window(sdl_ambulant_window* sdlas)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::set_ambulant_window(%p), m_ambulant_window=(sdl_ambulant_window*)%p", this, sdlas);
	// Don't destroy!
	//if (m_ambulant_window != NULL)
	//	delete m_ambulant_window;
	m_ambulant_window = sdlas;
	lib::rect r = m_bounds;
	m_lock.leave();
	need_redraw(r);
}

sdl_ambulant_window*
ambulant_sdl_window::get_sdl_ambulant_window()
{
//	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::get_sdl_ambulant_window(%p) returns=(sdl_ambulant_window*)%p",this,m_ambulant_window);
	return m_ambulant_window;
}

void
ambulant_sdl_window::set_gui_player(gui_player* gpl)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::set_gui_player(%p: %p)",this, (void *) gpl);
	m_lock.enter();
	m_gui_player = gpl;
	if (gpl != NULL && gpl->get_recorder_factory() != NULL) {
		m_recorder = gpl->get_recorder_factory()->new_recorder(net::pixel_argb, m_bounds.size());
	} else if (m_recorder != NULL) {
		delete m_recorder;
		m_recorder = NULL;
	}
	m_lock.leave();
}

gui_player*
ambulant_sdl_window::get_gui_player()
{
	return m_gui_player;
}

void
ambulant_sdl_window::resize_window (int w, int h) 
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::resize_window(%p): w=%d, h=%d", this, w, h);
	m_lock.enter();
	m_ambulant_window->sdl_resize_window (w, h);
	m_lock.leave();
	need_redraw(get_bounds());
}

#ifdef TBD
SDL_Surface*
ambulant_sdl_window::new_ambulant_surface()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::new_ambulant_surface(%p)",(void *)m_surface);
	if (m_surface != NULL) delete m_surface;
	return m_surface;
	m_lock.leave();
}

SDL_Surface*
ambulant_sdl_window::get_ambulant_oldsurface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::get_ambulant_oldsurface(%p) = %p",(void *)this,(void *)m_oldsurface);
	if (m_fullscreen_count && m_fullscreen_old_surface)
		return m_fullscreen_old_surface;
	return m_oldsurface;
}

SDL_Surface*
ambulant_sdl_window::get_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::get_ambulant_surface(%p) = %p",(void *)this,(void *)m_surface);
	return m_surface;
}

SDL_Surface*
ambulant_sdl_window::get_surface_from_screen(const lib::rect &r)
{
	return NULL;
}

void
ambulant_sdl_window::reset_ambulant_surface(void)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::reset_ambulant_surface(%p) m_oldsurface = %p",(void *)this,(void *)m_oldsurface);
	if (m_oldsurface != NULL) m_surface = m_oldsurface;
}

void
ambulant_sdl_window::set_ambulant_surface(SDL_Surface* surf)
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::set_ambulant_surface(%p) surf = %p",(void *)this,(void *)surf);
	m_oldsurface = m_surface;
	if (surf != NULL) m_surface = surf;
	m_lock.leave();
}

void
ambulant_sdl_window::delete_ambulant_surface()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::delete_ambulant_surface(%p) m_surface = %p",(void *)this, (void *)m_surface);
	m_lock.leave();
}
#endif//TBD

//
// sdl_ambulant_window
//
long unsigned int sdl_ambulant_window::s_num_events = 0;
int sdl_ambulant_window::s_windows = 0;
lib::critical_section sdl_ambulant_window::s_lock;
std::map<int, sdl_ambulant_window*>  sdl_ambulant_window::s_id_sdl_ambulant_window_map;

sdl_ambulant_window::sdl_ambulant_window(SDL_Window* window)
  :	m_screenshot_data(NULL),
	m_screenshot_size(0),
	m_ambulant_sdl_window(NULL),
	m_sdl_window(NULL),
	m_sdl_window_renderer(NULL),
	m_sdl_surface(NULL),
	m_sdl_transition_surface(NULL),
	m_sdl_renderer(NULL),
	m_sdl_screen_renderer(NULL),
	m_sdl_screen_surface(NULL),
	m_sdl_window_flags(0),
	m_sdl_fullscreen(false),
	m_sdl_scale(1.0),
	m_document_rect(SDL_Rect()),
	m_evp(NULL),
	m_screen_pixels(NULL),
	m_need_window_resize(false),
	m_new_width(0),
	m_new_height(0),
	m_window_pixel_format(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window.sdl_ambulant_window(%p): window=(SDL_Window*)%p", this, window);
	m_sdl_dst_rect.x = m_sdl_dst_rect.y = m_sdl_dst_rect.w = m_sdl_dst_rect.h = 0;
	// Environment variable SDL_WINDOW_FLAGS=<int> controls the flags given to SDL_CreateWindow
	char* sdl_window_flags_env = getenv("SDL_WINDOW_FLAGS");
	if (sdl_window_flags_env != NULL) {
		m_sdl_window_flags = atoi (sdl_window_flags_env);
		if (m_sdl_window_flags & SDL_WINDOW_OPENGL) {
			if (SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, 24)
			    || SDL_GL_SetAttribute (SDL_GL_STENCIL_SIZE, 8)
			    || SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1)
//			       SDL_GL_SetAttribute (SDL_GL_SWAP_CONTROL, 0)
			    || SDL_GL_SetSwapInterval(0)
			    ) {
			  m_sdl_window_flags &= ~SDL_WINDOW_OPENGL;
			  lib::logger::get_logger()->trace("%s() failed: %s -- continuing without SDL_WINDOW_OPENGL",
							   "SDL_GL_SetAttribute",SDL_GetError());
			}
		}
		if (m_sdl_window_flags & SDL_WINDOW_FULLSCREEN) {
			m_sdl_fullscreen = true;
		}
	}
}

sdl_ambulant_window::~sdl_ambulant_window()
{
	sdl_ambulant_window::s_lock.enter();

	AM_DBG ambulant::lib::logger::get_logger()->debug("sdl_ambulant_window::~sdl_ambulant_window(x%x) sdl_ambulant_window::s_windows=%d sdl_ambulant_window::s_id_sdl_ambulant_window_map.size()=%d", this, sdl_ambulant_window::s_windows, sdl_ambulant_window::s_id_sdl_ambulant_window_map.size());
	sdl_ambulant_window::s_windows--;
	// erase corresponding entries in the maps
	if (m_sdl_renderer != NULL) {
		SDL_DestroyRenderer(m_sdl_renderer);
	}
	if (m_sdl_screen_surface != NULL) {
		AM_DBG lib::logger::get_logger()->debug("%s  SDL_FreeSurface(%p: m_sdl_screen_surface=%p)", __PRETTY_FUNCTION__, (void*)this, m_sdl_screen_surface);
		SDL_FreeSurface(m_sdl_screen_surface);
	}
	if (m_sdl_transition_surface != NULL) {
		AM_DBG lib::logger::get_logger()->debug("%s  SDL_FreeSurface(%p: m_sdl_transition_surface=%p)", __PRETTY_FUNCTION__, (void*)this, m_sdl_transition_surface);
		SDL_FreeSurface(m_sdl_transition_surface);
	}
	if(m_screen_pixels != NULL) {
		free(m_screen_pixels);
	}
	if (m_sdl_window != NULL) {
		SDL_DestroyWindow(m_sdl_window);
	}
	sdl_ambulant_window::s_lock.leave();
}

void
sdl_ambulant_window::remove_redraw_SDL_Events(ambulant_sdl_window* asw) 
{
	// Remove any outstanding SDL_Events from the SDL Event Queue that contain 'this'
	// but leave all others
	SDL_Event* events = (SDL_Event*) malloc (sdl_ambulant_window::s_num_events*sizeof(SDL_Event)), * events_left =  (SDL_Event*) NULL;
	if (events == NULL) {
		return;
	}					      
	int n_events_left = 0;
	SDL_PeepEvents(events, sdl_ambulant_window::s_num_events, SDL_GETEVENT, SDL_USEREVENT, SDL_USEREVENT);
	for (int i = 0; i < sdl_ambulant_window::s_num_events; i++) {
		if (events[i].user.data1 != asw) {
			if (events_left == NULL) {
				events_left =  (SDL_Event*) malloc (sdl_ambulant_window::s_num_events*sizeof(SDL_Event));
				assert (events_left);
			}
			events_left[n_events_left++] = events[i];
		}
	}
	if (events_left != NULL) {
		// restore the events with type SDL_USEREVENT that not contain 'this'
		// events of other types get higher priority because of this, but the
		// other events of type SDL_USEREVENT will remain in order.  To fix this,
		// use a different SDL_EventType >= SDL_USEREVENT for each object
		SDL_PeepEvents(events_left, n_events_left, SDL_ADDEVENT, SDL_USEREVENT, SDL_USEREVENT);
		free (events_left);
	}
	free (events);
	sdl_ambulant_window::s_num_events = n_events_left;
}

void
sdl_ambulant_window::need_redraw (ambulant_sdl_window* asw, lib::rect r)
{
	SDL_Event e;
	lib::rect* redraw_rect = (lib::rect*) malloc (sizeof(lib::rect));
	*redraw_rect = r;
	e.user.code = 317107; // magic number
	e.type = SDL_USEREVENT;
	e.user.data1 = (void*) asw;
	e.user.data2 = (void*) redraw_rect;
	SDL_PushEvent(&e);
	sdl_ambulant_window::s_num_events++;
//	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::need_redraw(%p): SDL_PushEvent called r=(%d,%d,%d,%d) e={type=%d user.code=%d user.data1=%p user.data2=%p}", this,r.left(),r.top(),r.width(),r.height(), e.type, e.user.code, e.user.data1, e.user.data2);
}

void
sdl_ambulant_window::redraw (lib::rect r)
{
	sdl_ambulant_window::s_num_events--;
#ifdef  WITH_SDL_TEXTURE
	SDL_Renderer* renderer = get_sdl_window_renderer();
	SDL_RenderSetClipRect(renderer, NULL);
	SDL_SetRenderDrawColor (renderer, 0, 0, 0, 255 /*alpha*/);
	SDL_RenderClear(renderer);
//	saw->dump_sdl_surface (surface, "redr");
	SDL_Surface* screen_surface = get_sdl_surface();
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, screen_surface);	
	SDL_Rect sdl_dst_rect =  get_sdl_dst_rect();
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::redraw(%p) screen_surface=(SDL_Surface*)%p, renderer=(SDL_Renderer*)%p, texture=(SDL_Texture*)%p, sdl_dst_rect={%d,%d,%d,%d}", this, screen_surface, renderer, texture, sdl_dst_rect.x, sdl_dst_rect.y, sdl_dst_rect.w, sdl_dst_rect.h);
	if (texture == NULL) {
		return;
	}
	int err = SDL_RenderCopy(renderer, texture, NULL, &sdl_dst_rect);	
	assert (err==0);
	SDL_RenderPresent(renderer);
	SDL_DestroyTexture(texture);
#else// WITH_SDL_TEXTURE
	// this works just like SDL_Flip in SDL 1.3 (from: SDL_GetWindowSurface wiki)
	SDL_UpdateWindowSurface(get_sdl_window());
#endif//WITH_SDL_TEXTURE
}

const char*
sdl_ambulant_window::get_screen_pixels()
{
	SDL_Surface* sdl_screen = get_sdl_surface();
	if (sdl_screen == NULL) {
		return NULL;
	} else {
		return (const char*) sdl_screen->pixels;
	}
}

SDL_Rect
sdl_ambulant_window::compute_sdl_dst_rect(int w, int h, SDL_Rect r)
{
	SDL_DisplayMode sdm;
	lib::size sdl_screen_size = (w != 0 && h != 0) ? lib::size (w, h) : lib::size (r.w, r.h);
	m_sdl_dst_rect = r;
	if (m_sdl_fullscreen && w == 0 && h == 0) {
		int err = SDL_GetDesktopDisplayMode(0, &sdm);
		if (err != 0) lib::logger::get_logger()->trace("%s: %s", __PRETTY_FUNCTION__, SDL_GetError());
		sdl_screen_size = lib::size (sdm.w, sdm.h);
	} else {
		if (m_sdl_window != NULL) {
			SDL_GetWindowSize(m_sdl_window, (int*) &sdl_screen_size.w, (int*) &sdl_screen_size.h);
		}
	}
	AM_DBG lib::logger::get_logger()->debug("%s, %s=(%d,%d) r=(%d,%d, %d,%d)", "compute_sdl_dst_rect", "sdl_screen_size", 
						sdl_screen_size.w, sdl_screen_size.h, r.x, r.y, r.w, r.h);

	float xscale = float(sdl_screen_size.w) / r.w;
	float yscale = float(sdl_screen_size.h) / r.h;
	m_sdl_scale = xscale > yscale ? yscale : xscale;
	m_sdl_dst_rect.w = r.w * m_sdl_scale;
	m_sdl_dst_rect.h = r.h * m_sdl_scale;
	if (sdl_screen_size.w != m_sdl_dst_rect.w) {
		m_sdl_dst_rect.x = (sdl_screen_size.w - m_sdl_dst_rect.w)/2;
	}
	if (sdl_screen_size.h != m_sdl_dst_rect.h) {
		m_sdl_dst_rect.y = (sdl_screen_size.h - m_sdl_dst_rect.h)/2;
	}
	AM_DBG lib::logger::get_logger()->debug("%s, %s=(%d,%d, %d,%d), m_sdl_scale=%f", "compute_sdl_dst_rect", "m_sdl_dst_rect",
						m_sdl_dst_rect.x, m_sdl_dst_rect.y, m_sdl_dst_rect.w, m_sdl_dst_rect.h, m_sdl_scale);
	return m_sdl_dst_rect;
} 

int
sdl_ambulant_window::create_sdl_window_and_renderers(const char* window_name, lib::rect r)
{
	int err = 0;
	if (m_sdl_window != NULL) {
		return err;
	}
	AM_DBG lib::logger::get_logger()->trace("sdl_gui::create_sdl_window_and_renderers(%p): m_window=(SDL_Window*)%p, window ID=%d", this, m_sdl_window, SDL_GetWindowID(m_sdl_window));
	SDL_Rect sr = compute_sdl_dst_rect(0, 0, SDL_Rect_from_ambulant_rect(r));
	m_sdl_window = SDL_CreateWindow(window_name, sr.x, sr.y, sr.w, sr.h, m_sdl_window_flags);
	if (m_sdl_window == NULL) {
		SDL_SetError("Out of memory");
		err = -1;
	}
	if (err != 0) {
		return err;
	}
	assert (m_sdl_window);
	m_document_rect = SDL_Rect_from_ambulant_rect(r);
//XX	SDL_GLContext glcontext = SDL_GL_CreateContext(m_sdl_window);
//XX	SDL_SetWindowFullscreen(m_sdl_window, SDL_WINDOW_RESIZABLE);
	SDL_Rect sdl_rect = { r.left(),r.top(),r.width(),r.height() };
#ifdef  WITH_SDL_TEXTURE
	err = create_sdl_surface_and_pixels(&sdl_rect, &m_screen_pixels, &m_sdl_screen_surface, &m_sdl_screen_renderer);
#else //WITH_SDL_TEXTURE
	m_sdl_screen_surface = SDL_GetWindowSurface (m_sdl_window);
	if (m_sdl_screen_surface == NULL) {
		err = -1;
	}
#endif//WITH_SDL_TEXTURE
	if (err != 0) {
		return err;
	}
	m_sdl_surface = m_sdl_screen_surface;
	lib::logger::get_logger()->trace ("%s: m_sdl_screen_surface.format=%s", __PRETTY_FUNCTION__, SDL_GetPixelFormatName(m_sdl_screen_surface->format->format));
	m_window_pixel_format = m_sdl_screen_surface->format;

	// Everything OK, register the window for use by SDL_Loop in the embedder (it should have one)
	sdl_ambulant_window::s_lock.enter();
	sdl_ambulant_window::s_windows++;

	// The screen_renderer is special, it is the rendering context for the window pixels instead the surface pixels
#ifdef  WITH_SDL_TEXTURE
	if (m_sdl_window_renderer == NULL) {
		m_sdl_window_renderer = SDL_CreateRenderer(m_sdl_window, -1, SDL_RENDERER_ACCELERATED);
		if (m_sdl_window_renderer == NULL) {
			lib::logger::get_logger()->trace("No accelerated renderer,software renderer fallback", this);
			m_sdl_window_renderer = SDL_CreateRenderer(m_sdl_window, -1, SDL_RENDERER_SOFTWARE);
			if (m_sdl_window_renderer == NULL) {
				sdl_ambulant_window::s_lock.leave();
				lib::logger::get_logger()->warn("Cannot open: %s, error: %s for window %d", "SDL CreateRenderer", SDL_GetError(), SDL_GetWindowID(m_sdl_window));
				return -1;
			}
		}
	}
	m_sdl_renderer = m_sdl_screen_renderer; //TMP
#endif//WITH_SDL_TEXTURE
	Uint32 win_ID = SDL_GetWindowID (m_sdl_window);
	sdl_ambulant_window::s_id_sdl_ambulant_window_map[(int)win_ID] = this;
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::create_sdl_window_and_renderers::(%p): m_sdl_surface=(SDL_Surface*)%p m_sdl_renderer=(SDL_Renderer*)%p win_ID=%u sdl_ambulant_window::s_id_sdl_ambulant_window_map[win_ID]=%p", this, m_sdl_surface, m_sdl_renderer, win_ID, sdl_ambulant_window::s_id_sdl_ambulant_window_map[win_ID]);
	sdl_ambulant_window::s_lock.leave();
	return err;
}

int
sdl_ambulant_window::create_sdl_surface_and_pixels(SDL_Rect* r, uint8_t** pixels, SDL_Surface** surface, SDL_Renderer** renderer)
{
	int err = 0;
	if (pixels != NULL) {
		*pixels = (uint8_t*) malloc( r->w * r->h * SDL_BPP);
	}
	// From SDL documentation
 	/* Create a 32-bit surface with the bytes of each pixel in R,G,B,A order,
 	   as expected by OpenGL for textures */
 	
 	/* SDL interprets each pixel as a 32-bit number, so our masks must depend
 	   on the endianness (byte order) of the machine */
	// In ffmpeg_video, we use ARGB instead.
 #if SDL_BYTEORDER == SDL_BIG_ENDIAN
	Uint32 rmask=0xff000000, gmask = 0x00ff0000, bmask = 0x0000ff00, amask = 0x000000ff;
 #else
	Uint32 rmask=0x000000ff, gmask = 0x0000ff00, bmask = 0x00ff0000, amask = 0xff000000;
 #endif	
	if (surface != NULL) {
		if (pixels != NULL) {
// Using the following call will create a surface that fails to produce a proper SDL_Texture* later
// using SDL_CreateTextureFromSurface(), although this function does not indicate an error.
// The alternative using the default masks does not show this problem. Bug in SDL2 ?? 
// disabled:		*surface = SDL_CreateRGBSurfaceFrom(*pixels, r->w, r->h, 32, r->w*SDL_BPP, rmask, gmask, bmask, amask);
			if (*surface == NULL) {
				/* or using the default masks for the depth: */
				*surface = SDL_CreateRGBSurfaceFrom(*pixels, r->w, r->h, 32, r->w*SDL_BPP, 0, 0, 0, 0);
			}
		} else {
			*surface = SDL_CreateRGBSurface(0, r->w, r->h, r->w*SDL_BPP, rmask, gmask, bmask, amask);
			if (*surface == NULL) {
				SDL_CreateRGBSurface(0, r->w, r->h, r->w*SDL_BPP, 0, 0, 0,0);
			}
		}
		err = SDL_SetSurfaceBlendMode (*surface, SDL_BLENDMODE_BLEND);
		if (err != 0) {
			return err;
		}
	}
	lib::logger::get_logger()->trace ("%s: *surface->format=%s", __PRETTY_FUNCTION__, SDL_GetPixelFormatName((*surface)->format->format));

	if (renderer != NULL) {
		*renderer = SDL_CreateSoftwareRenderer(*surface);
		// enable alpha blending
		err = SDL_SetRenderDrawBlendMode(*renderer, SDL_BLENDMODE_BLEND);
	}
	return err;
}

void
sdl_ambulant_window::sdl_resize_window (int w, int h) 
{
	// called from sdl event loop, register values and schedule
	// the actual resizing to be done during next redraw
	m_new_width = w;
	m_new_height = h;
	m_need_window_resize = true;
	remove_redraw_SDL_Events(m_ambulant_sdl_window);
}

lib::point
sdl_ambulant_window::transform (SDL_Point p) 
{
	lib::point q(p.x, p.y);
	
	SDL_Rect sr = get_sdl_dst_rect();
	q.x = round((p.x - sr.x) / m_sdl_scale);
	q.y = round((p.y - sr.y) / m_sdl_scale);
	return q;
}

sdl_ambulant_window*
sdl_ambulant_window::get_sdl_ambulant_window(Uint32 windowID)
{
	sdl_ambulant_window* rv = NULL;
	sdl_ambulant_window::s_lock.enter();
	if ( ! s_id_sdl_ambulant_window_map.empty()) {
		rv = s_id_sdl_ambulant_window_map[windowID];
	}
	sdl_ambulant_window::s_lock.leave();
	return rv;
}

void
sdl_ambulant_window::set_ambulant_sdl_window( ambulant_sdl_window* asw)
{
	// Note: the window and window are destucted independently.
	//	if (m_sdl_window != NULL)
	//	  delete m_sdl_window;
	m_ambulant_sdl_window = asw;
	if (asw != NULL) {
		create_sdl_window_and_renderers("SDL2 Video_Test", asw->get_bounds());
	}
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::set_sdl_window(%p): m_ambulant_sdl_window=(ambulant_sdl_window*)%p, m_sdl_window=(SDL_Window*)%p)",
											this, m_ambulant_sdl_window, m_sdl_window);
}

// Transitions

void
sdl_ambulant_window::startScreenTransition()
{
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::startScreenTransition()");
	if (m_fullscreen_count)
		logger::get_logger()->trace("%s:multiple Screen transitions in progress (m_fullscreen_count=%d)","sdl_ambulant_window::startScreenTransition()",m_fullscreen_count);
	m_fullscreen_count++;
#ifdef TBD
	if (m_fullscreen_old_surface) g_object_unref(G_OBJECT(m_fullscreen_old_surface));
	m_fullscreen_old_surface = m_fullscreen_prev_surface;
	m_fullscreen_prev_surface = NULL;
#endif//TBD
}

void
sdl_ambulant_window::endScreenTransition()
{
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::endScreenTransition()");
	assert(m_fullscreen_count > 0);
	m_fullscreen_count--;
}

void
sdl_ambulant_window::screenTransitionStep(smil2::transition_engine* engine, lib::transition_info::time_type now)
{
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::screenTransitionStep()");
	assert(m_fullscreen_count > 0);
#ifdef TBD
	m_fullscreen_engine = engine;
	m_fullscreen_now = now;
#endif//TBD
}

void
sdl_ambulant_window::_screenTransitionPreRedraw()
{
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::_screenTransitionPreRedraw()");
	if (m_fullscreen_count == 0) return;
	// XXX setup drawing to transition window
}

void
sdl_ambulant_window::_screenTransitionPostRedraw(const lib::rect &r)
{
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::_screenTransitionPostRedraw()");
#ifdef TBD
	if (m_fullscreen_count == 0 && m_fullscreen_old_surface == NULL) {
		// Neither in fullscreen transition nor wrapping one up.
		// Take a snapshot of the screen and return.
		AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::_screenTransitionPostRedraw: screen snapshot");
		if (m_fullscreen_prev_surface) g_object_unref(G_OBJECT(m_fullscreen_prev_surface));
		m_fullscreen_prev_surface = get_surface_from_screen(r); // XXX wrong
		DUMPSURFACE(m_fullscreen_prev_surface, "snap");
		return;
	}
	if (m_fullscreen_old_surface == NULL) {
		// Just starting a new fullscreen transition. Get the
		//	bits from the snapshot saved during the previous
		// redraw.
		m_fullscreen_old_surface = m_fullscreen_prev_surface;
		m_fullscreen_prev_surface = NULL;
	}
#endif//TBD

	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::_screenTransitionPostRedraw: bitblit");
#ifdef  TBD
	if (m_fullscreen_engine) {
		// Do the transition step
		SDL_Surface<* new_src = get_ambulant_window();
		if ( ! new_src) new_src = new_ambulant_window();
		gdk_surface_bitblt(m_window, 0, 0, m_surface, r.left(), r.top(), r.width(), r.height());
		gdk_surface_bitblt(m_surface, 0, 0, m_fullscreen_old_surface, r.left(), r.top(), r.width(), r.height());
		DUMPSURFACE(new_src, "fnew");
		DUMPSURFACE(m_surface, "fold");
		m_fullscreen_engine->step(m_fullscreen_now);
		DUMPSURFACE(m_surface, "fres");
	}
#endif//TBD

	if (m_fullscreen_count == 0) {
		// Finishing a fullscreen transition.
		AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::_screenTransitionPostRedraw: cleanup after transition done");
#ifdef TBD
		if (m_fullscreen_old_surface) g_object_unref(G_OBJECT(m_fullscreen_old_surface));
		m_fullscreen_old_surface = NULL;
		m_fullscreen_engine = NULL;
#endif//TBD
	}
}

bool
sdl_ambulant_window::clear_sdl_surface (lib::rect r)
// helper: on clear the surface
{
	bool redraw_needed = false;
	if (m_need_window_resize) {
		m_need_window_resize = false;
		SDL_Renderer* sdl_window_renderer = get_sdl_window_renderer();
		SDL_RenderSetClipRect(sdl_window_renderer, NULL);
		SDL_SetRenderDrawColor (sdl_window_renderer, 0, 0, 0, 255 /*alpha*/);
		SDL_RenderClear(sdl_window_renderer);
		SDL_RenderPresent(sdl_window_renderer);
		remove_redraw_SDL_Events(m_ambulant_sdl_window);
		SDL_Rect sdl_rect = compute_sdl_dst_rect (m_new_width, m_new_height, m_document_rect);
		AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::clear_SDL_Surface(%p) sdl_window_renderer= %p, sdl_rect={%d,%d,%d,%d}", this, sdl_window_renderer, sdl_rect.x, sdl_rect.y, sdl_rect.w, sdl_rect.h);
		redraw_needed = true;
	} else {
		SDL_Rect sdl_rect = SDL_Rect_from_ambulant_rect(r); 
		SDL_Surface* sdl_surface = get_sdl_surface();
		AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::clear_SDL_Surface(%p) = %p, sdl_rect={%d,%d,%d,%d}", this, sdl_surface, sdl_rect.x, sdl_rect.y, sdl_rect.w, sdl_rect.h);
		// Fill with <brush> color
		color_t color = lib::to_color(255, 255, 255);

		AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::clear_SDL_Surface(%p): clearing to %p", this, (long)color);
		Uint32 sdl_color = SDL_MapRGBA(sdl_surface->format, redc(color), greenc(color), bluec(color), 255);
		SDL_SetClipRect(sdl_surface, &sdl_rect);
		SDL_FillRect(sdl_surface, &sdl_rect, sdl_color);
	}
	return redraw_needed;
}

SDL_Surface*
sdl_ambulant_window::copy_sdl_surface(SDL_Surface* surface)
{
	SDL_Surface* new_surface = SDL_ConvertSurface(surface, surface->format, surface->flags);
	return new_surface;
}

void
sdl_ambulant_window::push_sdl_surface(SDL_Surface* new_surface)
{
	m_transition_surfaces.push(new_surface);
}

void 
sdl_ambulant_window::set_sdl_surface(SDL_Surface* s) {
	if (s != NULL) {
		m_sdl_surface = s;
	}
}

void 
sdl_ambulant_window::delete_sdl_surface() {
	if (m_sdl_surface != NULL) {
		SDL_FreeSurface (m_sdl_surface);
	}
	m_sdl_surface = NULL;
}

SDL_Surface*
sdl_ambulant_window::get_transition_surface()
{
	if (m_sdl_transition_surface == NULL) {
		m_sdl_transition_surface = copy_sdl_surface (m_sdl_screen_surface);
	}
	SDL_Surface* s = m_sdl_transition_surface;
	assert(s && s->format && s->map && s->refcount > 0);
	return m_sdl_transition_surface;
}

void
sdl_ambulant_window::delete_transition_surface()
{
	if (m_sdl_transition_surface != NULL) {
		SDL_FreeSurface (m_sdl_screen_surface);
		m_sdl_transition_surface = NULL;
	}
}

int
  _copy_sdl_surface (SDL_Surface* src, SDL_Rect* src_rect, SDL_Surface* dst, SDL_Rect* dst_rect, Uint8 alpha, SDL_Rect* clip_rect)
{
	int rv = 0;
//	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::copy_sdl_surface(): src=%p src_rect={%d,%d,%d,%d} dst=%p dst_rect={%d,%d,%d,%d} alpha=%u", src, src_rect?src_rect->x:0, src_rect?src_rect->y:0, src_rect?src_rect->w:0, src_rect?src_rect->h:0, dst,  dst_rect->x, dst_rect->y, dst_rect->w, dst_rect->h, alpha);
	// Check requirements for SDL blitting
	return rv;
}

int
sdl_ambulant_window::copy_to_sdl_surface (SDL_Surface* src, SDL_Rect* src_rect, SDL_Rect* dst_rect, Uint8 alpha, SDL_Rect* clip_rect)
{
	int rv = 0;
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::copy_to_sdl_surface(): dst_rect={%d,%d %d,%d} alpha=%u", dst_rect->x, dst_rect->y, dst_rect->w, dst_rect->h, alpha);
	SDL_Surface* dst = get_sdl_surface();
	if (src == NULL || dst == NULL || src->format == NULL || dst->format == NULL) {
		// Can't do anything
		return rv;
	}
	sdl_ambulant_window::s_lock.enter();
	bool dst_locked = false;
	bool src_locked = false;
	bool must_scale = false;
	if (SDL_MUSTLOCK(dst)) {
		rv = SDL_LockSurface(dst);
		if (rv >= 0) {
			dst_locked = true;
		}
	}
	if (rv >= 0 && SDL_MUSTLOCK(src)) {
		rv = SDL_LockSurface(src);
		if (rv >= 0) {
			src_locked = true;
		}
	}
	SDL_Rect sdl_src_rect;
	if (src_rect == NULL) {
		sdl_src_rect.x = sdl_src_rect.w = 0;
		sdl_src_rect.w = src->w;
		sdl_src_rect.h = src->h;
	} else {
		sdl_src_rect = *src_rect;
	}
	if (sdl_src_rect.w != dst_rect->w || sdl_src_rect.h != dst_rect->h) {
		must_scale = true;
	}
	SDL_Rect sdl_old_clip_rect;
	if (clip_rect != NULL) {
		SDL_GetClipRect(dst, &sdl_old_clip_rect);
		SDL_SetClipRect(dst, clip_rect);
	}
	if (rv >= 0) {
		rv = SDL_SetSurfaceAlphaMod (src, alpha);
		if (rv < 0) {
			lib::logger::get_logger()->debug("sdl_ambulant_window::copy_sdl_surface(): error from %s: %s", "SDL_SetSurfaceAlphaMod", SDL_GetError());
		}
	}
	if (src->format->format != dst->format->format) {
		AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::copy_sdl_surface(): srcformat=%s dstformat=%s", SDL_GetPixelFormatName(src->format->format), SDL_GetPixelFormatName(dst->format->format));
	}
	SDL_Surface* tmp_surface = NULL, *dst_surface = dst, *src_surface = src;
	void* pixels = NULL;
	SDL_Rect tmp_rect = sdl_src_rect;
	if (rv >= 0) {
		if (must_scale) {
			if (dst_rect->w > dst->w || dst_rect->h > dst->h) {
				// must create large enough surface for scaled blit
				tmp_surface = SDL_CreateRGBSurface (0, dst_rect->w, dst_rect->h, 32, 0, 0, 0, 0); 
				if (tmp_surface != NULL) {
					pixels = malloc (dst_rect->w*dst_rect->h*4);
					SDL_FreeFormat (tmp_surface->format);
					tmp_surface->format = dst->format;
					dst->format->refcount++;
				}
				if (tmp_surface == NULL || pixels == NULL) {
		    			rv = SDL_SetError("Out of memory");
				} else {
					src_surface = dst_surface = tmp_surface;
					float h_scale = (float) dst_rect->h / (float) sdl_src_rect.h;
					float w_scale = (float) dst_rect->w / (float) sdl_src_rect.w;
					tmp_rect.x = (int) roundf(sdl_src_rect.x*w_scale);
					tmp_rect.y = (int) roundf(sdl_src_rect.y*h_scale);
					tmp_rect.w = (int) roundf(sdl_src_rect.w*w_scale);
					tmp_rect.h = (int) roundf(sdl_src_rect.h*h_scale);
				}
			}
		}
	}
	if (rv >= 0 && must_scale) {
//		dump_sdl_surface (src, "src");
		rv = SDL_BlitScaled(src, &sdl_src_rect, dst_surface, dst_rect);
//		dump_sdl_surface (dst_surface, tmp_surface ? "tmp" : "dst");
	}
	if (rv >= 0 && ! (must_scale && tmp_surface == NULL)) {
		rv = SDL_BlitSurface(src_surface, &tmp_rect, dst, dst_rect);
//		dump_sdl_surface (dst, "dst");
	}
	if (rv < 0) {
		lib::logger::get_logger()->debug("sdl_ambulant_window::copy_sdl_surface(): error from %s: %s", must_scale ? "SDL_BlitScaled" :"SDL_BlitSurface", SDL_GetError());
	}
	if (pixels != NULL) {
	    free (pixels);
	}
	if (tmp_surface != NULL) {
		SDL_FreeSurface (tmp_surface);
	}
	if (clip_rect != NULL) {
		SDL_SetClipRect(dst, &sdl_old_clip_rect);
	}
	if (src_locked) {
		SDL_UnlockSurface(src);
	}
	if (dst_locked) {
		SDL_UnlockSurface(dst);
	}
	sdl_ambulant_window::s_lock.leave();
	return rv;
}

void
sdl_ambulant_window::dump_sdl_surface (SDL_Surface* surf, const char* id)
{
	if (surf == NULL || id == NULL || m_evp == NULL) {
		return;
	}
	char filename[256];
	sprintf(filename,"%%%.8lu%s.bmp", m_evp->get_timer()->elapsed(), id);
	SDL_SaveBMP(surf, filename);
}

void
sdl_ambulant_window::dump_sdl_renderer (SDL_Renderer* renderer, const SDL_Rect r, const char* id)
{
	if (renderer == NULL || id == NULL) {
		return;
	}
	void* pixels = malloc(r.w*r.h*SDL_BPP);
	int err = pixels ? 0 : -1;
	if (err == 0) {
		err = SDL_RenderReadPixels (renderer, &r, SDL_PIXELFORMAT_RGBA4444, pixels, r.w*SDL_BPP);
	}
	SDL_Surface* s = NULL;
	if (err == 0) {
 #if SDL_BYTEORDER == SDL_BIG_ENDIAN
		Uint32 rmask=0xff000000, gmask = 0x00ff0000, bmask = 0x0000ff00, amask = 0x000000ff;
 #else
		Uint32 rmask=0x000000ff, gmask = 0x0000ff00, bmask = 0x00ff0000, amask = 0xff000000;
 #endif	
		s = SDL_CreateRGBSurfaceFrom(pixels, r.w, r.h, 32, r.w*SDL_BPP, rmask, gmask, bmask, amask);
		if (s == NULL) {
		  err = -1;
		}
	}
	if (err == 0) {
		dump_sdl_surface (s, id);
	}
	if (s != NULL) {
		SDL_FreeSurface (s);
	}
	if (pixels != NULL) {
		free (pixels);
	}
	assert(err==0);
}

#ifdef TBD
void
sdl_ambulant_window::do_paint_event (GdkEventExpose *e) {
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::paintEvent(%p): e=%p)", (void*) this, (void*) e);
	lib::rect r = lib::rect(
		lib::point(e->area.x, e->area.y),
		lib::size(e->area.width, e->area.height));
	if (m_sdl_window == NULL) {
		lib::logger::get_logger()->debug("sdl_ambulant_window::paintEvent(%p): e=%p m_sdl_window==NULL",
			(void*) this, (void*) e);
		return;
	}
	m_sdl_window->redraw(r);
}

void
sdl_ambulant_window::do_motion_notify_event(GdkEventMotion *e) {
	int m_o_x = 0, m_o_y = 0; //27; // XXXX Origin of MainWindow
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::mouseMoveEvent(%p) e=(%ld,%ld) m_sdl_window=%p\n", this, e->x,e->y, m_sdl_window);
	if (! m_sdl_window) return;
	//XXXX This is not right!!!
	ambulant::lib::point ap = ambulant::lib::point((int)e->x, (int)e->y);
	gui_player* gui_player =  m_sdl_window->get_gui_player();
	if (gui_player) {
		gui_player->before_mousemove(0);
		m_sdl_window->user_event(ap, 1);
	}
	int cursid = 0;
	if (gui_player)
		cursid = gui_player->after_mousemove();

	// Set hand cursor if cursid==1, arrow if cursid==0.
	GdkCursor* cursor;
	// gdk cursors need to be cached by the window factory
	cursor = cursid == 0 
		? m_sdl_window->get_gdk_cursor(GDK_ARROW)
		: m_sdl_window->get_gdk_cursor(GDK_HAND1);
	if (cursor)
		gdk_window_set_cursor (m_window->window, cursor);
}

void
sdl_ambulant_window::do_button_release_event(GdkEventButton *e) {
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::do_button_release_event(%p): e=%p, position=(%d, %d))", (void*) this, (void*) e, e->x, e->y);
	if (m_sdl_window == NULL) {
		lib::logger::get_logger()->debug("sdl_ambulant_window::do_button_release_event(%p): e=%p  position=(%d, %d) m_sdl_window==NULL", (void*) this, (void*) e, e->x, e->y);
		return;
	}
	if (e->type == GDK_BUTTON_RELEASE){
		lib::point amwhere = lib::point((int)e->x, (int)e->y);
		m_sdl_window->user_event(amwhere);
	}
}

void
sdl_ambulant_window::do_key_release_event(GdkEventKey *e) {
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::do_key_release_event(%p): e=%p  key=%d, length=%d string=%s) m_sdl_window==NULL", (void*) this, (void*) e, e->keyval, e->length, e->string);
	if (m_sdl_window == NULL) {
		lib::logger::get_logger()->debug("sdl_ambulant_window::do_key_release_event(%p): e=%p  key=%d, length=%d string=%s) m_sdl_window==NULL", (void*) this, (void*) e, e->keyval, e->length, e->string);
		return;
	}
	if (e->type == GDK_KEY_RELEASE){
		lib::logger::get_logger()->debug("sdl_ambulant_window::do_key_release_event(%p): e=%p  key=%d, length=%d string=%s) m_sdl_window==NULL", (void*) this, (void*) e, e->keyval, e->length, e->string);
//		m_sdl_window->user_event(amwhere);
		m_sdl_window->get_gui_player()->on_char((int) *e->string);
	}
}

void sdl_ambulant_window::get_size(int *width, int *height){
	gdk_drawable_get_size(m_window->window, width, height);
}

bool sdl_ambulant_window::get_screenshot(const char *type, char **out_data, size_t *out_size){
	*out_data = NULL;
	*out_size = 0;
#ifndef WITH_SCREENSHOTS
	lib::logger::get_logger()->error("get_screenshot: no support for screenshots");
#endif // WITH_SCREENSHOTS
	*out_data= m_screenshot_data;
	*out_size = m_screenshot_size;
	return TRUE;
}

// Not implemented
bool sdl_ambulant_window::set_overlay(const char *type, const char *data, size_t size){
	return FALSE;
}

// Not implemented
bool sdl_ambulant_window::clear_overlay(){
	return FALSE;
}


bool sdl_ambulant_window::set_screenshot(char **screenshot_data, size_t *screenshot_size){
	return TRUE;
}
#endif//TBD


#endif//WITH_SDL2

