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

#ifdef  WITH_SDL2 // work in progress

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/gui/SDL/sdl_factory.h"
#include "ambulant/gui/SDL/sdl_fill.h"
#include "ambulant/gui/SDL/sdl_window.h"
//X #include "ambulant/gui/SDL/sdl_includes.h"
//X #include "ambulant/gui/SDL/sdl_image_renderer.h"
//X #include "ambulant/gui/SDL/sdl_smiltext.h"
//X #include "ambulant/gui/SDL/sdl_util.h"
//X #include "ambulant/gui/SDL/sdl_text_renderer.h"
//X #include "ambulant/gui/SDL/sdl_video_renderer.h"

#include "SDL.h"

using namespace ambulant;
using namespace gui::sdl;
using namespace net;


common::window_factory *
//X ambulant::gui::sdl::create_sdl_window_factory(sdl_ambulant_window* sdl_window, gui_player* gpl)
ambulant::gui::sdl::create_sdl_window_factory(void* sdl_window, common::gui_player* gpl) // unsafe
{
	return new sdl_window_factory(sdl_window, gpl);
}

#ifdef JNK

common::window_factory *
ambulant::gui::sdl::create_sdl_window_factory_unsafe(void* sdl_parent_window, common::gui_player* gpl)
{
	SdlWindow *parent = reinterpret_cast<SdlWindow*>(sdl_parent_window);
	if (parent == NULL) {
		lib::logger::get_logger()->fatal("create_sdl_window_factory: Cannot cast parent_window to SdlWindow");
		return NULL;
	}
	sdl_ambulant_window *sdlw = new sdl_ambulant_window(parent);
	if (sdlw == NULL) {
		lib::logger::get_logger()->fatal("create_sdl_window_factory: Cannot create sdl_ambulant_window");
		return NULL;
	}
	return new sdl_window_factory(sdlw, gpl);
}

#endif//JNK

//
// sdl_window_factory
//

//X sdl_window_factory::sdl_window_factory( sdl_ambulant_window* parent_window, common::gui_player* gpl)
sdl_window_factory::sdl_window_factory(void* parent_window, common::gui_player* gpl)
:	m_gui_player(gpl),
	m_parent_window(parent_window)
{
	AM_DBG lib::logger::get_logger()->debug("sdl_window_factory (0x%x)", (void*) this);
}

sdl_window_factory::~sdl_window_factory( )
{
	AM_DBG lib::logger::get_logger()->debug("~sdl_window_factory (0x%x)", (void*) this);
}

common::gui_window *
sdl_window_factory::new_window (const std::string &name, lib::size bounds, common::gui_events *region)
{
	lib::rect r (m_p, bounds);
	AM_DBG lib::logger::get_logger()->debug("sdl_window_factory::new_window (0x%x): name=%s %d,%d,%d,%d", (void*) this, name.c_str(), r.left(),r.top(),r.right(),r.bottom());
	ambulant_sdl_window * asdlw = new ambulant_sdl_window(name, &r, region);
	sdl_ambulant_window * sdlaw = (sdl_ambulant_window*) m_parent_window;
	if (sdlaw == NULL) {
		m_parent_window = sdlaw = new sdl_ambulant_window(NULL);
		assert (sdlaw);
	}
	asdlw->set_ambulant_window(sdlaw);
	sdlaw->set_ambulant_sdl_window(asdlw);
#ifdef JNK
	// We don't create this window anymore MainLoop does it!!
	sdl_window_set_size_request(m_parent_window->get_sdl_window(), r.right(), r.bottom());
	sdl_window_set_size_request(
		sdl_window_get_toplevel (m_parent_window->get_sdl_window()),
		r.right(),
		r.bottom()+ 25);
	sdl_window_show(m_parent_window->get_sdl_window());
//	sdl_ambulant_window * sdlaw = new sdl_ambulant_window(name, r, m_parent_window);

	// Wrong!!! I need to add the GUI size
//	sdl_window_set_size_request(SDL_WINDOW (sdl_window_get_toplevel (sdlaw->get_sdl_window())), r.right(), r.bottom()+ 25);
//	asdlw->set_ambulant_window(sdlaw);
//	sdlaw->set_sdl_window(asdlw);
	AM_DBG lib::logger::get_logger()->debug("sdl_window_factory::new_window(0x%x): ambulant_window=0x%x sdl_window=0x%x", (void*) this, (void*) m_parent_window, (void*) asdlw);
	return asdlw;
#endif//JNK
	SDL_Rect sdl_rect;
	sdl_rect.x = r.left();
	sdl_rect.y = r.top();
	sdl_rect.w = r.width();
	sdl_rect.h = r.height();
	sdl_ambulant_window* saw = (sdl_ambulant_window*) m_parent_window;
	ambulant_sdl_window* asw = saw->get_ambulant_sdl_window();
	return asdlw;
}

common::bgrenderer *
sdl_window_factory::new_background_renderer(const common::region_info *src)
{
	AM_DBG lib::logger::get_logger()->debug("sdl_window_factory::new_background_renderer(0x%x): src=0x%x", (void*) this, src);
#ifdef WITH_SDL_IMAGE
	return new ambulant::gui::sdl::sdl_background_renderer(src);
#else
	return NULL;
#endif // WITH_SDL_IMAGE
}

#ifdef JNK

// structure to keep track of the dirty area
struct dirty_area_window {
	lib::rect area;
	SdlWindow* window;
	sdl_ambulant_window* ambulant_window;
	guint tag;
};

// Callbacks used to keep synchronization of the different threads
extern "C" {
bool sdl_C_callback_helper_queue_draw_area(void *arg)
{
	assert(arg);
	dirty_area_window *r = (dirty_area_window *)arg;
	assert(r != 0);
	AM_DBG ambulant::lib::logger::get_logger()->debug("sdl_C_callback_helper_queue_draw_area with left: %d, top: %d, width: %d, height: %d s_windows=%d", r->area.left(), r->area.top(), r->area.width(), r->area.height(),sdl_ambulant_window::s_windows);
	sdl_ambulant_window::s_lock.enter();
	if (sdl_ambulant_window::s_windows > 0)
		sdl_window_queue_draw_area(r->window,
			r->area.left(), r->area.top(), r->area.width(), r->area.height());

	AM_DBG ambulant::lib::logger::get_logger()->debug("sdl_C_callback_helper_queue_draw_area with left: %d, top: %d, width: %d, height: %d tag=%d", r->area.left(), r->area.top(), r->area.width(), r->area.height(),r->tag);

	r->ambulant_window->m_draw_area_tags.erase(r->tag);
	sdl_ambulant_window::s_lock.leave();
//	sdl_window_queue_draw(r->window);
	delete r;
	return false;
//	r->release();
}
}


extern "C" {
void sdl_C_callback_do_paint_event(void *userdata, GdkEventExpose *event, SdlWindow *window)
{
	((sdl_ambulant_window*) userdata)->do_paint_event(event);
}
}

extern "C" {
void sdl_C_callback_do_motion_notify_event(void *userdata, GdkEventMotion *event, SdlWindow *window)
{
	((sdl_ambulant_window*) userdata)->do_motion_notify_event(event);
}
}

extern "C" {
void sdl_C_callback_do_button_release_event(void *userdata, GdkEventButton *event, SdlWindow *window)
{
	gint s_x, s_y, d_x, d_y;
	sdl_ambulant_window* gaw = (sdl_ambulant_window*) userdata;
	GdkWindow* s_gdkw; /* source window */
	SdlWindow* d_sdlw; /* destination window */
	SdlWindow* t_sdlw; /* toplevel window */

	// Find in the window stack the window in which window
	// the event source coordinates (s_x,s_y) are defined
	
	if (gaw == NULL || event == NULL)
		return; /* cannot handle */
	s_x = (gint) round (event->x);
	s_y = (gint) round (event->y);
	s_gdkw = event->window;

	d_sdlw = gaw->get_sdl_window ();
	t_sdlw = sdl_window_get_toplevel (d_sdlw);

	for (SdlWindow* a_sdlw = d_sdlw;
		a_sdlw != NULL;
		a_sdlw = sdl_window_get_parent (a_sdlw))
	{
		if (s_gdkw == a_sdlw->window) {
			/* found corresponding GdkWindow in SdlWindow stack */
			/* translate if necessary */
			if (a_sdlw != d_sdlw
				&& sdl_window_translate_coordinates (a_sdlw, d_sdlw, s_x, s_y, &d_x, &d_y))
			{
				event->x = d_x;
				event->y = d_y;
			}
			gaw->do_button_release_event (event);
			break;
		}
		if (a_sdlw == t_sdlw)
			break; /* not found */
	}
}
}//extern "C"


extern "C" {
void sdl_C_callback_do_key_release_event(void *userdata, GdkEventKey *event, SdlWindow *window)
{
	sdl_ambulant_window* gaw = (sdl_ambulant_window*) userdata;

	// Find in the window stack the window in which window
	// the event source coordinates (s_x,s_y) are defined
	
	if (gaw == NULL || event == NULL)
		return; /* cannot handle */
	gaw ->do_key_release_event (event);
}
}//extern "C"

void gui::sdl::gdk_pixmap_bitblt(
	GdkPixmap* dst, int dst_x, int dst_y,
	GdkPixmap* src, int src_x, int src_y,
	int width, int height)
{
	GdkGC *gc = gdk_gc_new (dst);
	gdk_draw_pixmap(GDK_DRAWABLE(dst), gc, GDK_DRAWABLE(src), src_x, src_y, dst_x, dst_y, width, height);
	g_object_unref (G_OBJECT (gc));
};


//
// sdl_window_factory
//

sdl_window_factory::sdl_window_factory( sdl_ambulant_window* parent_window, common::gui_player* gpl)
:	m_gui_player(gpl),
	m_parent_window(parent_window)
{
	AM_DBG lib::logger::get_logger()->debug("sdl_window_factory (0x%x)", (void*) this);
	m_arrow_cursor = gdk_cursor_new(GDK_ARROW);
	m_hand1_cursor = gdk_cursor_new(GDK_HAND1);
	m_hand2_cursor = gdk_cursor_new(GDK_HAND2);
}

sdl_window_factory::~sdl_window_factory( )
{
	gdk_cursor_unref(m_arrow_cursor);
	gdk_cursor_unref(m_hand1_cursor);
	gdk_cursor_unref(m_hand2_cursor);
}

common::gui_window *
sdl_window_factory::new_window (const std::string &name, lib::size bounds, common::gui_events *region)
{
	lib::rect r (m_p, bounds);
	AM_DBG lib::logger::get_logger()->debug("sdl_window_factory::new_window (0x%x): name=%s %d,%d,%d,%d", (void*) this, name.c_str(), r.left(),r.top(),r.right(),r.bottom());
	ambulant_sdl_window * asdlw = new ambulant_sdl_window(name, &r, region);

	// We don't create this window anymore MainLoop does it!!
	sdl_window_set_size_request(m_parent_window->get_sdl_window(), r.right(), r.bottom());
	sdl_window_set_size_request(
		sdl_window_get_toplevel (m_parent_window->get_sdl_window()),
		r.right(),
		r.bottom()+ 25);
	sdl_window_show(m_parent_window->get_sdl_window());
//	sdl_ambulant_window * sdlaw = new sdl_ambulant_window(name, r, m_parent_window);

	// Wrong!!! I need to add the GUI size
//	sdl_window_set_size_request(SDL_WINDOW (sdl_window_get_toplevel (sdlaw->get_sdl_window())), r.right(), r.bottom()+ 25);
//	asdlw->set_ambulant_window(sdlaw);
	set_ambulant_window(m_parent_window);
	m_parent_window->set_ambulant_sdl_window(asdlw);
//	sdlaw->set_sdl_window(asdlw);
	AM_DBG lib::logger::get_logger()->debug("sdl_window_factory::new_window(0x%x): ambulant_window=0x%x sdl_window=0x%x", (void*) this, (void*) m_parent_window, (void*) asdlw);
	asdlw->set_gui_player(m_gui_player);
	asdlw->set_gdk_cursor(GDK_ARROW, m_arrow_cursor);
	asdlw->set_gdk_cursor(GDK_HAND1, m_hand1_cursor);
	asdlw->set_gdk_cursor(GDK_HAND2, m_hand2_cursor);
	return asdlw;
}

common::bgrenderer *
sdl_window_factory::new_background_renderer(const common::region_info *src)
{
	AM_DBG lib::logger::get_logger()->debug("sdl_window_factory::new_background_renderer(0x%x): src=0x%x", (void*) this, src);
	return new sdl_background_renderer(src);
}



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
	m_oldpixmap(NULL),
	m_tmppixmap(NULL),
	m_arrow_cursor(NULL),
	m_hand1_cursor(NULL),
	m_hand2_cursor(NULL),
	m_fullscreen_count(0),
	m_fullscreen_prev_pixmap(NULL),
	m_fullscreen_old_pixmap(NULL),
	m_fullscreen_engine(NULL),
	m_fullscreen_now(0),
	m_window(NULL)
{
	m_pixmap = NULL;
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::ambulant_sdl_window(0x%x)",(void *)this);
}

ambulant_sdl_window::~ambulant_sdl_window()
{

	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::~ambulant_sdl_window(0x%x): m_ambulant_window=0x%x, m_pixmap=0x%x",this,m_ambulant_window, m_pixmap);
	// Note that we don't destroy the window, only sver the connection.
	// the window itself is destroyed independently.
	if (m_ambulant_window ) {
		m_ambulant_window->set_sdl_window(NULL);
//XXXX next delete Reload crashes with sdl, not with qt
//		delete m_ambulant_window;
		m_ambulant_window = NULL;
	}
	if (m_pixmap != NULL) {
		g_object_unref(G_OBJECT(m_pixmap));
		m_pixmap = NULL;
	}
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

void
ambulant_sdl_window::need_redraw(const lib::rect &r)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::need_redraw(0x%x): ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.width(), r.height());
	if (m_ambulant_window == NULL) {
		lib::logger::get_logger()->error("ambulant_sdl_window::need_redraw(0x%x): m_ambulant_window == NULL !!!", (void*) this);
		return;
	}
	// we use the parent window for redraw in case this window has been deleted at the time
	// the callback function is actually called (e.g. the user selects a different file)
	SdlWindow* this_window = m_ambulant_window->get_sdl_window();
	dirty_area_window* dirty = new dirty_area_window();
	dirty->window = (sdl_ambulant_window*) sdl_window_get_parent(this_window);
	dirty->area = r;
	if ( ! sdl_window_translate_coordinates (this_window, dirty->window, r.left(), r.top(), &dirty->area.x, &dirty->area.y)) {
		AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::need_redraw(0x%x): sdl_window_translate_coordinates failed.", (void *)this);
	}
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::need_redraw: parent ltrb=(%d,%d,%d,%d)", dirty->area.left(), dirty->area.top(), dirty->area.width(), dirty->area.height());
	dirty->ambulant_window = m_ambulant_window;
	sdl_ambulant_window::s_lock.enter();
	guint draw_area_tag = g_idle_add_full(G_PRIORITY_HIGH_IDLE, (GSourceFunc) sdl_C_callback_helper_queue_draw_area, (void *)dirty, NULL);
	dirty->tag = draw_area_tag;
	dirty->ambulant_window->m_draw_area_tags.insert(draw_area_tag);
	sdl_ambulant_window::s_lock.leave();
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::need_redraw: parent ltrb=(%d,%d,%d,%d), tag=%d fun=0x%x", dirty->area.left(), dirty->area.top(), dirty->area.width(), dirty->area.height(), draw_area_tag, sdl_C_callback_helper_queue_draw_area);
}

void
ambulant_sdl_window::redraw(const lib::rect &r)
{

	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::redraw(0x%x): ltrb=(%d,%d,%d,%d)",(void *)this, r.left(), r.top(), r.width(), r.height());
	_screenTransitionPreRedraw();
	clear();
	m_handler->redraw(r, this);
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
		printf (" Tenemos un error%s", error->message);
		g_error_free (error);
	}
	g_object_unref (G_OBJECT (pixbuf));
#endif //WITH_SCREENSHOTS
	DUMPPIXMAP(m_pixmap, "top");
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
ambulant_sdl_window::set_ambulant_window(sdl_ambulant_window* sdlaw)
{

	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::set_ambulant_window(0x%x)",(void *)sdlaw);
	// Don't destroy!
	//if (m_ambulant_window != NULL)
	//	delete m_ambulant_window;
	m_ambulant_window = sdlaw;
	GdkColor color;
	GdkColormap *cmap = gdk_colormap_get_system();

	if (sdlaw != NULL) {

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
//	}else{
//		sdl_window_hide(SDL_WINDOW (sdlaw->get_sdl_window()));
//		sdl_box_pack_start (SDL_BOX (parent_window), SDL_WINDOW (m_window), TRUE, TRUE, 0);
//		sdl_container_remove(SDL_CONTAINER (sdlaw->get_sdl_window()->parent), SDL_WINDOW (sdlaw->get_sdl_window()));
//		free(sdlaw->get_sdl_window());
//	}
}

sdl_ambulant_window*
ambulant_sdl_window::get_ambulant_window()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::get_ambulant_window(0x%x)",(void *)m_ambulant_window);
	return m_ambulant_window;
}

GdkPixmap*
ambulant_sdl_window::get_ambulant_pixmap()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::ambulant_pixmap(0x%x) = 0x%x",(void *)this,(void *)m_pixmap);
	return m_pixmap;
}

void
ambulant_sdl_window::set_gui_player(gui_player* gpl)
{
	m_gui_player = gpl;
}

gui_player*
ambulant_sdl_window::get_gui_player()
{
	return m_gui_player;
}

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

GdkPixmap*
ambulant_sdl_window::get_ambulant_oldpixmap()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::get_ambulant_oldpixmap(0x%x) = 0x%x",(void *)this,(void *)m_oldpixmap);
	if (m_fullscreen_count && m_fullscreen_old_pixmap)
		return m_fullscreen_old_pixmap;
	return m_oldpixmap;
}

GdkPixmap*
ambulant_sdl_window::get_ambulant_window()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::get_ambulant_window(0x%x) = 0x%x",(void *)this,(void *)m_window);
	return m_window;
}

GdkPixmap*
ambulant_sdl_window::get_pixmap_from_screen(const lib::rect &r)
{
	GdkPixmap *rv = gdk_pixmap_new(m_ambulant_window->get_sdl_window()->window, r.width(), r.height(), -1);
	gdk_pixmap_bitblt(rv, r.left(), r.top(), m_pixmap, r.left(), r.top(), r.width(), r.height());
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::get_pixmap_from_screen(0x%x) = 0x%x",(void *)this,(void *)m_pixmap);
	return rv;
}

void
ambulant_sdl_window::reset_ambulant_window(void)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::reset_ambulant_window(0x%x) m_oldpixmap = 0x%x",(void *)this,(void *)m_oldpixmap);
	if (m_oldpixmap != NULL) m_pixmap = m_oldpixmap;
}

void
ambulant_sdl_window::set_ambulant_window(GdkPixmap* surf)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::set_ambulant_window(0x%x) surf = 0x%x",(void *)this,(void *)surf);
	m_oldpixmap = m_pixmap;
	if (surf != NULL) m_pixmap = surf;
}

void
ambulant_sdl_window::delete_ambulant_window()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::delete_ambulant_window(0x%x) m_window = 0x%x",(void *)this, (void *)m_window);
	delete m_window;
	m_window = NULL;
}

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
		AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::_screenTransitionPostRedraw: cleanup after transition done");
		if (m_fullscreen_old_pixmap) g_object_unref(G_OBJECT(m_fullscreen_old_pixmap));
		m_fullscreen_old_pixmap = NULL;
		m_fullscreen_engine = NULL;
	}
}

void
ambulant_sdl_window::clear()
// private helper: clear the window
{
	// Fill with <brush> color
	if (m_pixmap == NULL) {
		AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::clear(): m_pixmap == NULL!!");
		return;
	}
	color_t color = lib::to_color(255, 255, 255);
	AM_DBG lib::logger::get_logger()->debug("ambulant_sdl_window::clear(): clearing to 0x%x", (long)color);
	GdkColor bgc;
	bgc.red = redc(color)*0x101;
	bgc.blue = bluec(color)*0x101;
	bgc.green = greenc(color)*0x101;
	GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (m_pixmap));
	gdk_gc_set_rgb_fg_color (gc, &bgc);
	gdk_draw_rectangle (GDK_DRAWABLE (m_pixmap), gc, TRUE,
		m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h);
	g_object_unref (G_OBJECT (gc));
}


//
// sdl_ambulant_window
//
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

sdl_ambulant_window::sdl_ambulant_window(SdlWindow* window)
:	m_sdl_window(NULL),
	m_screenshot_data(NULL),
	m_screenshot_size(0)
{
	m_window = window;
	GObject* ancestor_window = G_OBJECT (SDL_WINDOW (sdl_window_get_ancestor(m_window, SDL_TYPE_WINDOW)));

	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::sdl_ambulant_window(0x%x-0x%x) s_windows=%d",
		(void *)this,
		(void*) window, sdl_ambulant_window::s_windows);
	m_expose_event_handler_id = g_signal_connect_swapped (G_OBJECT (m_window),
		"expose_event", G_CALLBACK (sdl_C_callback_do_paint_event), (void*) this);
	m_motion_notify_handler_id = g_signal_connect_swapped (ancestor_window,
		"motion_notify_event", G_CALLBACK (sdl_C_callback_do_motion_notify_event), (void*) this);
	m_button_release_handler_id = g_signal_connect_swapped (ancestor_window,
		"button_release_event", G_CALLBACK (sdl_C_callback_do_button_release_event), (void*) this);
	m_key_release_handler_id = g_signal_connect_swapped (ancestor_window,
		"key_release_event", G_CALLBACK (sdl_C_callback_do_key_release_event), (void*) this);
	lib::logger::get_logger()->debug("sdl_ambulant_window::sdl_ambulant_window(0x%x-0x%x) m_key_release_handler_id=%0x%x", this, window, m_key_release_handler_id);
	sdl_window_add_events(SDL_WINDOW (ancestor_window),
		GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
	// window needs focus for receiving key press/release events
	SDL_WINDOW_SET_FLAGS(ancestor_window, SDL_CAN_FOCUS);
	sdl_window_grab_focus(SDL_WINDOW(ancestor_window));
	sdl_ambulant_window::s_lock.enter();
	sdl_ambulant_window::s_windows++;
	sdl_ambulant_window::s_lock.leave();
}

sdl_ambulant_window::~sdl_ambulant_window()
{
	sdl_ambulant_window::s_lock.enter();
	sdl_ambulant_window::s_windows--;
	if ( ! m_draw_area_tags.empty()) {
		for (std::set<guint>::iterator it = m_draw_area_tags.begin(); it != m_draw_area_tags.end(); it++) {
			AM_DBG ambulant::lib::logger::get_logger()->debug("sdl_ambulant_window::~sdl_ambulant_window removing tag %d", (*it));
			g_source_remove((*it));

		}
	}
	sdl_ambulant_window::s_lock.leave();
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
}

void
sdl_ambulant_window::set_sdl_window( ambulant_sdl_window* asdlw)
{
	// Note: the window and window are destucted independently.
	//	if (m_sdl_window != NULL)
	//	  delete m_sdl_window;
	m_sdl_window = asdlw;
	AM_DBG lib::logger::get_logger()->debug("sdl_ambulant_window::set_sdl_window(0x%x): m_sdl_window==0x%x)",
		(void*) this, (void*) m_sdl_window);
}

ambulant_sdl_window*
sdl_ambulant_window::sdl_window() {
	return m_sdl_window;
}

SdlWindow*
sdl_ambulant_window::get_sdl_window()
{
	return m_window;
}

void sdl_ambulant_window::do_paint_event (GdkEventExpose *e) {
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
