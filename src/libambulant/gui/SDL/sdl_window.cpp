/// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
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

#ifdef  WITH_SDL2 // work in prpgress

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
	m_sdl_surface(NULL),
	m_recorder(NULL),
	m_record(false)
//X	m_oldpixmap(NULL),
//X	m_tmppixmap(NULL),
//X	m_arrow_cursor(NULL),
//X	m_hand1_cursor(NULL),
//X	m_hand2_cursor(NULL),
//X	m_fullscreen_count(0),
//X	m_fullscreen_prev_pixmap(NULL),
//X	m_fullscreen_old_pixmap(NULL),
//X	m_fullscreen_engine(NULL),
//X	m_fullscreen_now(0),
//X	m_window(NULL)
{
//X	m_pixmap = NULL;
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::ambulant_sdl_window(0x%x)",(void *)this);
	m_pixels = (uint8_t*) malloc(bounds->width()*bounds->height()*SDL_BPP);
	// we use ARGB
	Uint32 amask=0xff000000, rmask = 0x00ff0000, gmask = 0x0000ff00, bmask = 0x000000ff;
	m_sdl_surface = SDL_CreateRGBSurfaceFrom(m_pixels, bounds->width(), bounds->height(), 32, bounds->width()*SDL_BPP, rmask, gmask, bmask, 0);
}
long unsigned int ambulant_sdl_window::s_num_events = 0;

ambulant_sdl_window::~ambulant_sdl_window()
{

	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::~ambulant_sdl_window(0x%x): m_ambulant_window=0x%x",this,m_ambulant_window);
	// Note that we don't destroy the window, only sver the connection.
	// the window itself is destroyed independently.
	if (m_ambulant_window ) {
		m_ambulant_window->set_ambulant_sdl_window(NULL);
		m_ambulant_window = NULL;
	}
	if (ambulant_sdl_window::s_num_events == 0) {
		return;
	}
	// Remove any outstanding SDL_Events from the SDL Event Queue that contain 'this'
	// but leave all others
	SDL_Event* events = (SDL_Event*) malloc (s_num_events*sizeof(SDL_Event)), * events_left =  (SDL_Event*) NULL;
	int n_events_left = 0;
	SDL_PeepEvents(events, ambulant_sdl_window::s_num_events, SDL_GETEVENT, SDL_USEREVENT, SDL_USEREVENT);
	for (int i = 0; i < ambulant_sdl_window::s_num_events; i++) {
		if (events[i].user.data1 != this) {
			if (events_left == NULL) {
				events_left =  (SDL_Event*) malloc (ambulant_sdl_window::s_num_events*sizeof(SDL_Event));
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
	ambulant_sdl_window::s_num_events = n_events_left;
	free(m_pixels);
	if (m_sdl_surface != NULL) {
		SDL_FreeSurface(m_sdl_surface);
	}
//X	if (m_pixmap != NULL) {
//X		g_object_unref(G_OBJECT(m_pixmap));
//X		m_pixmap = NULL;
//X	}
/** m_oldpixmap not to be deleted, it points to either m_pixmap or m_tmppixmap
	if (m_oldpixmap != NULL) {
		g_object_unref(G_OBJECT(m_oldpixmap));
		m_oldpixmap = NULL;
	}
**/
/** m_tmppixmap not to be deleted, it is not a copy anymore
	if (m_tmppixmap != NULL) {
		g_object_unref(G_OBJECT(m_tmppixmap));
		m_tmppixmap = NULL;
	}
**/
}
#ifdef JNK
void
ambulant_sdl_window::set_gdk_cursor(GdkCursorType gdk_cursor_type, GdkCursor* gdk_cursor)
{
	switch (gdk_cursor_type) {
	case GDK_ARROW: m_arrow_cursor = gdk_cursor;
	case GDK_HAND1: m_hand1_cursor = gdk_cursor;
	case GDK_HAND2: m_hand2_cursor = gdk_cursor;
	default:	return;
	}

}

GdkCursor*
ambulant_sdl_window::get_gdk_cursor(GdkCursorType gdk_cursor_type)
{
	switch (gdk_cursor_type) {
	case GDK_ARROW: return m_arrow_cursor;
	case GDK_HAND1: return m_hand1_cursor;
	case GDK_HAND2: return m_hand2_cursor;
	default:	return NULL;
	}
}
#endif//JNK

void
ambulant_sdl_window::need_redraw(const lib::rect &r)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::need_redraw(0x%x): ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.width(), r.height());
	if (m_ambulant_window == NULL) {
		lib::logger::get_logger()->error("ambulant_sdl_window::need_redraw(0x%x): m_ambulant_window == NULL !!!", (void*) this);
		return;
	}
#ifdef JNK
	// we use the parent window for redraw in case this window has been deleted at the time
	// the callback function is actually called (e.g. the user selects a different file)
	SDL_Window* this_window = m_ambulant_window->get_sdl_window();
	dirty_area_window* dirty = new dirty_area_window();
	dirty->window = (sdl_ambulant_window*) sdl_window_get_parent(this_window);
	dirty->area = r;
	if ( ! sdl_window_translate_coordinates (this_window, dirty->window, r.left(), r.top(), &dirty->area.x, &dirty->area.y)) {
		AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::need_redraw(0x%x): sdl_window_translate_coordinates failed.", (void *)this);
	}
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::need_redraw: parent ltrb=(%d,%d,%d,%d)", dirty->area.left(), dirty->area.top(), dirty->area.width(), dirty->area.height());
	dirty->ambulant_window = m_ambulant_window;
	sdl_ambulant_window::s_lock.enter();
//X	guint draw_area_tag = g_idle_add_full(G_PRIORITY_HIGH_IDLE, (GSourceFunc) sdl_C_callback_helper_queue_draw_area, (void *)dirty, NULL);
	dirty->tag = draw_area_tag;
	dirty->ambulant_window->m_draw_area_tags.insert(draw_area_tag);
	sdl_ambulant_window::s_lock.leave();
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::need_redraw: parent ltrb=(%d,%d,%d,%d), tag=%d fun=0x%x", dirty->area.left(), dirty->area.top(), dirty->area.width(), dirty->area.height(), draw_area_tag, sdl_C_callback_helper_queue_draw_area);
#endif//JNK
	//X as we don't have a SDL event loop yet, directly call redraw()
	//X redraw(r);//XXXX !!!!!
	//X SDL_Event* e = (SDL_Event*) malloc (sizeof SDL_Event);;
	static int hack = 1;
	SDL_Event e;
	m_redraw_rect = r;
	e.user.code = 317107; // magic number
	e.type = SDL_USEREVENT;
	e.user.data1 = (void*) this;
	e.user.data2 = (void*) 0;
	SDL_PushEvent(&e);
	ambulant_sdl_window::s_num_events++;
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::need_redraw(0x%x): SDL_PushEvent called r=(%d,%d,%d,%d) e={type=%d user.code=%d user.data1=0x%x user.data2=0x%x}", this,r.left(),r.top(),r.width(),r.height(), e.type, e.user.code, e.user.data1, e.user.data2);
}

void
ambulant_sdl_window::redraw(const lib::rect &r)
{

	ambulant_sdl_window::s_num_events--;
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::redraw(0x%x): ltrb=(%d,%d,%d,%d)",(void *)this, r.left(), r.top(), r.width(), r.height());
//X	_screenTransitionPreRedraw();
	clear();
	m_handler->redraw(r, this);
#ifdef JNK
//XXXX	if ( ! isEqualToPrevious(m_pixmap))
	_screenTransitionPostRedraw(r);
	gdk_pixmap_bitblt(
		m_ambulant_window->get_sdl_window()->window, r.left(), r.top(),
		m_pixmap, r.left(), r.top(),
		r.width(), r.height());
#ifdef WITH_SCREENSHOTS
	GError *error = NULL;
	gint width; gint height;

	gdk_drawable_get_size(m_ambulant_window->get_sdl_window()->window, &width, &height);

	GdkPixbuf* pixbuf = gdk_pixbuf_get_from_drawable(
		NULL,
		m_ambulant_window->get_sdl_window()->window,
		0, 0, 0, 0, 0, width, height);
//	if (!gdk_pixbuf_save_to_buffer (pixbuf, &buffer, &buffer_size, "jpeg", &error, "quality", "100", NULL)) {
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
	DUMPPIXMAP(m_pixmap, "top");
#endif//JNK
       	SDL_Rect rect;
       	rect.x = r.left();
	rect.y = r.top();
	rect.w = r.width();
	rect.h = r.height();
	SDL_Renderer* renderer = get_sdl_ambulant_window()->get_sdl_renderer();
	if (m_recorder) {
		char filename[256];
		sprintf(filename,"%%%0.16lu.bmp", get_sdl_ambulant_window()->get_evp()->get_timer()->elapsed());
		SDL_SaveBMP(get_sdl_surface(), filename);
	}
	{
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, get_sdl_surface());		
		AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::redraw(0x%x) rect={%d,%d,%d,%d}", this,
						 rect.x, rect.y, rect.w, rect.h);
		SDL_RenderCopy(renderer, texture, NULL, NULL);	
		SDL_RenderPresent(renderer);
		SDL_DestroyTexture(texture);
	}
}

void
ambulant_sdl_window::redraw_now()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::redraw_now()");
}

bool
ambulant_sdl_window::user_event(const lib::point &where, int what)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::user_event(0x%x): point=(%d,%d)", this, where.x, where.y);
	return m_handler->user_event(where, what);
}

void
ambulant_sdl_window::need_events(bool want)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::need_events(0x%x): want=%d", this, want);
}

void
ambulant_sdl_window::set_ambulant_window(sdl_ambulant_window* sdlas)
{

	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::set_ambulant_window(0x%x)",(void *)sdlas);
	// Don't destroy!
	//if (m_ambulant_window != NULL)
	//	delete m_ambulant_window;
	m_ambulant_window = sdlas;

#ifdef JNK
	if (sdlas != NULL) {
		GdkColor color;
		GdkColormap *cmap = gdk_colormap_get_system();
		// color is white
		gdk_color_parse("white", &color);

		// in debugging mode, initialize with purple background
		AM_DBG gdk_color_parse("Purple", &color);
		gdk_colormap_alloc_color(cmap, &color, FALSE, TRUE);

		// set the color in the window
		sdl_window_modify_bg (SDL_WINDOW (sdlaw->get_sdl_window()), SDL_STATE_NORMAL, &color );

		// Initialize m_pixmap
		gint width; gint height;
		sdl_window_get_size_request(SDL_WINDOW (sdlaw->get_sdl_window()), &width, &height);
		m_pixmap = gdk_pixmap_new(sdlaw->get_sdl_window()->window, width, height, -1);
		AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::set_ambulant_window(0x%x); size (%i,%i)",(void *)sdlaw, width, height);
		// User Interaction
	}
#endif//JNK
}

sdl_ambulant_window*
ambulant_sdl_window::get_sdl_ambulant_window()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::get_sdl_ambulant_window(0x%x)",(void *)m_ambulant_window);
	return m_ambulant_window;
}

#ifdef JNK
GdkPixmap*
ambulant_sdl_window::get_ambulant_pixmap()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::ambulant_pixmap(0x%x) = 0x%x",(void *)this,(void *)m_pixmap);
	return m_pixmap;
}
#endif//JNK

void
ambulant_sdl_window::set_gui_player(gui_player* gpl)
{
	m_gui_player = gpl;
	if (gpl != NULL && gpl->get_recorder_factory() != NULL) {
		m_recorder = gpl->get_recorder_factory()->new_recorder();
	} else if (m_recorder != NULL) {
		delete m_recorder;
		m_recorder = NULL;
	}
}

gui_player*
ambulant_sdl_window::get_gui_player()
{
	return m_gui_player;
}

#ifdef JNK
GdkPixmap*
ambulant_sdl_window::new_ambulant_window()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::new_ambulant_window(0x%x)",(void *)m_window);
	if (m_window != NULL) delete m_window;
	gint width; gint height;
	gdk_drawable_get_size(GDK_DRAWABLE (m_pixmap), &width, &height);
	m_window = gdk_pixmap_new(m_pixmap, width, height, -1);
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::new_ambulant_window(0x%x)",(void *)m_window);
	return m_window;
}
#endif//JNK

#ifdef JNK
GdkPixmap*
ambulant_sdl_window::get_ambulant_oldpixmap()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::get_ambulant_oldpixmap(0x%x) = 0x%x",(void *)this,(void *)m_oldpixmap);
	if (m_fullscreen_count && m_fullscreen_old_pixmap)
		return m_fullscreen_old_pixmap;
	return m_oldpixmap;
}
#endif//JNK

#ifdef JNK
GdkPixmap*
ambulant_sdl_window::get_ambulant_window()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::get_ambulant_window(0x%x) = 0x%x",(void *)this,(void *)m_window);
	return m_window;
}
#endif//JNK

#ifdef JNK
GdkPixmap*
ambulant_sdl_window::get_pixmap_from_screen(const lib::rect &r)
{
	GdkPixmap *rv = gdk_pixmap_new(m_ambulant_window->get_sdl_window()->window, r.width(), r.height(), -1);
	gdk_pixmap_bitblt(rv, r.left(), r.top(), m_pixmap, r.left(), r.top(), r.width(), r.height());
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::get_pixmap_from_screen(0x%x) = 0x%x",(void *)this,(void *)m_pixmap);
	return rv;
}
#endif//JNK

#ifdef JNK
void
ambulant_sdl_window::reset_ambulant_window(void)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::reset_ambulant_window(0x%x) m_oldpixmap = 0x%x",(void *)this,(void *)m_oldpixmap);
	if (m_oldpixmap != NULL) m_pixmap = m_oldpixmap;
}
#endif//JNK

#ifdef JNK
void
ambulant_sdl_window::set_ambulant_window(GdkPixmap* surf)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::set_ambulant_window(0x%x) surf = 0x%x",(void *)this,(void *)surf);
	m_oldpixmap = m_pixmap;
	if (surf != NULL) m_pixmap = surf;
}
#endif//JNK

#ifdef JNK
void
ambulant_sdl_window::delete_ambulant_window()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::delete_ambulant_window(0x%x) m_window = 0x%x",(void *)this, (void *)m_window);
	delete m_window;
	m_window = NULL;
}
#endif//JNK

#ifdef JNK

// Transitions

void
ambulant_sdl_window::startScreenTransition()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::startScreenTransition()");
	if (m_fullscreen_count)
		logger::get_logger()->trace("%s:multiple Screen transitions in progress (m_fullscreen_count=%d)","ambulant_sdl_window::startScreenTransition()",m_fullscreen_count);
	m_fullscreen_count++;
	if (m_fullscreen_old_pixmap) g_object_unref(G_OBJECT(m_fullscreen_old_pixmap));
	m_fullscreen_old_pixmap = m_fullscreen_prev_pixmap;
	m_fullscreen_prev_pixmap = NULL;
}

void
ambulant_sdl_window::endScreenTransition()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::endScreenTransition()");
	assert(m_fullscreen_count > 0);
	m_fullscreen_count--;
}

void
ambulant_sdl_window::screenTransitionStep(smil2::transition_engine* engine, lib::transition_info::time_type now)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::screenTransitionStep()");
	assert(m_fullscreen_count > 0);
	m_fullscreen_engine = engine;
	m_fullscreen_now = now;
}

void
ambulant_sdl_window::_screenTransitionPreRedraw()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::_screenTransitionPreRedraw()");
	if (m_fullscreen_count == 0) return;
	// XXX setup drawing to transition window
}

void
ambulant_sdl_window::_screenTransitionPostRedraw(const lib::rect &r)
{

	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::_screenTransitionPostRedraw()");
	if (m_fullscreen_count == 0 && m_fullscreen_old_pixmap == NULL) {
		// Neither in fullscreen transition nor wrapping one up.
		// Take a snapshot of the screen and return.
		AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::_screenTransitionPostRedraw: screen snapshot");
		if (m_fullscreen_prev_pixmap) g_object_unref(G_OBJECT(m_fullscreen_prev_pixmap));
		m_fullscreen_prev_pixmap = get_pixmap_from_screen(r); // XXX wrong
		DUMPPIXMAP(m_fullscreen_prev_pixmap, "snap");
		return;
	}
	if (m_fullscreen_old_pixmap == NULL) {
		// Just starting a new fullscreen transition. Get the
		//	bits from the snapshot saved during the previous
		// redraw.
		m_fullscreen_old_pixmap = m_fullscreen_prev_pixmap;
		m_fullscreen_prev_pixmap = NULL;
	}

	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::_screenTransitionPostRedraw: bitblit");
	if (m_fullscreen_engine) {
		// Do the transition step
		GdkPixmap* new_src = get_ambulant_window();
		if ( ! new_src) new_src = new_ambulant_window();
		gdk_pixmap_bitblt(m_window, 0, 0, m_pixmap, r.left(), r.top(), r.width(), r.height());
		gdk_pixmap_bitblt(m_pixmap, 0, 0, m_fullscreen_old_pixmap, r.left(), r.top(), r.width(), r.height());
		DUMPPIXMAP(new_src, "fnew");
		DUMPPIXMAP(m_pixmap, "fold");
		m_fullscreen_engine->step(m_fullscreen_now);
		DUMPPIXMAP(m_pixmap, "fres");
	}

	if (m_fullscreen_count == 0) {
		// Finishing a fullscreen transition.
\		AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::_screenTransitionPostRedraw: cleanup after transition done");
		if (m_fullscreen_old_pixmap) g_object_unref(G_OBJECT(m_fullscreen_old_pixmap));
		m_fullscreen_old_pixmap = NULL;
		m_fullscreen_engine = NULL;
	}
}
#endif//JNK

void
ambulant_sdl_window::clear()
// private helper: clear the window
{
	// Fill with <brush> color
#ifdef JNK
	if (m_pixmap == NULL) {
		AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::clear(): m_pixmap == NULL!!");
		return;
	}
#endif//JNK
	color_t color = lib::to_color(255, 255, 255);
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::clear(): clearing to 0x%x", (long)color);
#ifdef JNK
	GdkColor bgc;
	bgc.red = redc(color)*0x101;
	bgc.blue = bluec(color)*0x101;
	bgc.green = greenc(color)*0x101;
	GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (m_pixmap));
	gdk_gc_set_rgb_fg_color (gc, &bgc);
	gdk_draw_rectangle (GDK_DRAWABLE (m_pixmap), gc, TRUE,
		m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h);
	g_object_unref (G_OBJECT (gc));
#endif//JNK
}

int
ambulant_sdl_window::copy_sdl_surface (SDL_Surface* src, SDL_Rect* src_rect, SDL_Rect* dst_rect)
{
	int rv = 0;
	if (src != NULL && dst_rect != NULL) {
//		dump_sdl_surface (m_sdl_surface, "befor"); 
 		rv = SDL_BlitSurface(src, src_rect, m_sdl_surface, dst_rect);
//		dump_sdl_surface (m_sdl_surface, "after"); 
	}
	return rv;
}

void
ambulant_sdl_window::dump_sdl_surface (SDL_Surface* surf, const char* id)
{
		char filename[256];
		sprintf(filename,"%%%0.8lu%s.bmp", get_sdl_ambulant_window()->get_evp()->get_timer()->elapsed(), id);
		SDL_SaveBMP(surf, filename);
}

//
// sdl_ambulant_window
//
//*** The following comment is inherited from gtk_factory and probably garbage
// sdl_ambulant_window::s_windows is a counter to check for the liveliness of sdl_window during
// execution of sdl*draw() functions by a callback function in the main thread
// sdl_ambulant_window::s_lock is for the protection of the counter
// TBD: a better approach would be to have s static protected std::vector<dirty_window>
// to be updated when callbacks are scheduled and executed
// and use these entries to remove any scheduled callbacks with
// gboolean g_idle_remove_by_data (gpointer data); when the sdl_window is destroyed
// then the ugly dependence on the parent window couls also be removed
int sdl_ambulant_window::s_windows = 0;
lib::critical_section sdl_ambulant_window::s_lock;
std::map<SDL_Window*, SDL_Renderer*>  sdl_ambulant_window::s_window_map;

sdl_ambulant_window::sdl_ambulant_window(SDL_Window* window)
  :	m_screenshot_data(NULL),
	m_screenshot_size(0)
{
	m_sdl_window = window;
	sdl_ambulant_window::s_lock.enter();
	sdl_ambulant_window::s_windows++;
	sdl_ambulant_window::s_lock.leave();
	m_sdl_renderer = s_window_map[window];
	if (m_sdl_renderer == NULL) {
		m_sdl_renderer = SDL_CreateRenderer(/*asw->window()*/ window, -1, SDL_RENDERER_ACCELERATED);
		if (m_sdl_renderer == NULL) {
			AM_DBG lib::logger::get_logger()->trace("sdl_ambulant_window.sdl_ambulant_window(0x%x): trying software renderer", this);
			m_sdl_renderer = SDL_CreateRenderer(/*asw->window()*/ window, -1, SDL_RENDERER_SOFTWARE);
			if (m_sdl_renderer == NULL) {
				lib::logger::get_logger()->warn("Cannot open: %s, error: %s", "SDL Createrenderer", SDL_GetError());
				return;
			}
		}
		s_window_map[window] = m_sdl_renderer;
	}
}

sdl_ambulant_window::~sdl_ambulant_window()
{
	sdl_ambulant_window::s_lock.enter();
	sdl_ambulant_window::s_windows--;
#ifdef JNK
	if ( ! m_draw_area_tags.empty()) {
		for (std::set<guint>::iterator it = m_draw_area_tags.begin(); it != m_draw_area_tags.end(); it++) {
			AM_DBG ambulant::lib::logger::get_logger()->debug("sdl_ambulant_window::~sdl_ambulant_window removing tag %d", (*it));
			g_source_remove((*it));

		}
	}
#endif//JNK
	sdl_ambulant_window::s_lock.leave();
#ifdef JNK
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::~sdl_ambulant_window(0x%x): m_sdl_window=0x%x s_windows=%d", (void*)this, m_sdl_window, sdl_ambulant_window::s_windows);
	GObject* ancestor_window = G_OBJECT (SDL_WINDOW (sdl_window_get_ancestor(m_window, SDL_TYPE_WINDOW)));
	if (g_signal_handler_is_connected (G_OBJECT (m_window), m_expose_event_handler_id))
		g_signal_handler_disconnect(G_OBJECT (m_window), m_expose_event_handler_id);
	if (g_signal_handler_is_connected (ancestor_window, m_motion_notify_handler_id))
		g_signal_handler_disconnect(ancestor_window, m_motion_notify_handler_id);
	if (g_signal_handler_is_connected (ancestor_window, m_button_release_handler_id))
		g_signal_handler_disconnect(ancestor_window, m_button_release_handler_id);
	if (g_signal_handler_is_connected (ancestor_window, m_key_release_handler_id))
		g_signal_handler_disconnect(ancestor_window, m_key_release_handler_id);
	if (m_sdl_window) {
		m_sdl_window->set_ambulant_window(NULL);
		m_sdl_window = NULL;
	}
	if (m_screenshot_data) {
		g_free(m_screenshot_data);
		m_screenshot_data = NULL;
		m_screenshot_size = 0;
	}
#endif//JNK
}

void
sdl_ambulant_window::set_ambulant_sdl_window( ambulant_sdl_window* asdlw)
{
	// Note: the window and window are destucted independently.
	//	if (m_sdl_window != NULL)
	//	  delete m_sdl_window;
	m_ambulant_sdl_window = asdlw;
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::set_sdl_window(0x%x): m_sdl_window==0x%x)",
		(void*) this, (void*) m_sdl_window);
}

#ifdef JNK
void
sdl_ambulant_window::do_paint_event (GdkEventExpose *e) {
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::paintEvent(0x%x): e=0x%x)", (void*) this, (void*) e);
	lib::rect r = lib::rect(
		lib::point(e->area.x, e->area.y),
		lib::size(e->area.width, e->area.height));
	if (m_sdl_window == NULL) {
		lib::logger::get_logger()->debug("sdl_ambulant_window::paintEvent(0x%x): e=0x%x m_sdl_window==NULL",
			(void*) this, (void*) e);
		return;
	}
	m_sdl_window->redraw(r);
}

void
sdl_ambulant_window::do_motion_notify_event(GdkEventMotion *e) {
	int m_o_x = 0, m_o_y = 0; //27; // XXXX Origin of MainWindow
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::mouseMoveEvent(0x%x) e=(%ld,%ld) m_sdl_window=0x%x\n", this, e->x,e->y, m_sdl_window);
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
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::do_button_release_event(0x%x): e=0x%x, position=(%d, %d))", (void*) this, (void*) e, e->x, e->y);
	if (m_sdl_window == NULL) {
		lib::logger::get_logger()->debug("sdl_ambulant_window::do_button_release_event(0x%x): e=0x%x  position=(%d, %d) m_sdl_window==NULL", (void*) this, (void*) e, e->x, e->y);
		return;
	}
	if (e->type == GDK_BUTTON_RELEASE){
		lib::point amwhere = lib::point((int)e->x, (int)e->y);
		m_sdl_window->user_event(amwhere);
	}
}

void
sdl_ambulant_window::do_key_release_event(GdkEventKey *e) {
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::do_key_release_event(0x%x): e=0x%x  key=%d, length=%d string=%s) m_sdl_window==NULL", (void*) this, (void*) e, e->keyval, e->length, e->string);
	if (m_sdl_window == NULL) {
		lib::logger::get_logger()->debug("sdl_ambulant_window::do_key_release_event(0x%x): e=0x%x  key=%d, length=%d string=%s) m_sdl_window==NULL", (void*) this, (void*) e, e->keyval, e->length, e->string);
		return;
	}
	if (e->type == GDK_KEY_RELEASE){
		lib::logger::get_logger()->debug("sdl_ambulant_window::do_key_release_event(0x%x): e=0x%x  key=%d, length=%d string=%s) m_sdl_window==NULL", (void*) this, (void*) e, e->keyval, e->length, e->string);
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

#endif//JNK

#endif//WITH_SDL2

