// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2012 Stichting CWI, 
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

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/gui/gtk/gtk_factory.h"
#include "ambulant/gui/gtk/gtk_includes.h"
#include "ambulant/gui/gtk/gtk_image_renderer.h"
#ifdef	WITH_GTK_HTML_WIDGET
#include "ambulant/gui/gtk/gtk_html_renderer.h"
#endif/*WITH_GTK_HTML_WIDGET*/
#include "ambulant/gui/gtk/gtk_smiltext.h"
#include "ambulant/gui/gtk/gtk_util.h"
#include "ambulant/gui/gtk/gtk_text_renderer.h"
#include "ambulant/gui/gtk/gtk_video_renderer.h"

using namespace ambulant;
using namespace gui::gtk;
using namespace net;


common::window_factory *
ambulant::gui::gtk::create_gtk_window_factory(gtk_ambulant_widget* gtk_widget, gui_player* gpl)
{
	return new gtk_window_factory(gtk_widget, gpl);
}

common::window_factory *
ambulant::gui::gtk::create_gtk_window_factory_unsafe(void* gtk_parent_widget, common::gui_player* gpl)
{
	GtkWidget *parent = reinterpret_cast<GtkWidget*>(gtk_parent_widget);
	if (parent == NULL) {
		lib::logger::get_logger()->fatal("create_gtk_window_factory: Cannot cast parent_widget to GtkWidget");
		return NULL;
	}
	gtk_ambulant_widget *gtkw = new gtk_ambulant_widget(parent);
	if (gtkw == NULL) {
		lib::logger::get_logger()->fatal("create_gtk_window_factory: Cannot create gtk_ambulant_widget");
		return NULL;
	}
	return new gtk_window_factory(gtkw, gpl);
}


// structure to keep track of the dirty area
struct dirty_area_widget {
	lib::rect area;
	GtkWidget* widget;
	gtk_ambulant_widget* ambulant_widget;
	guint tag;
};

// Callbacks used to keep synchronization of the different threads
extern "C" {
bool gtk_C_callback_helper_queue_draw_area(void *arg)
{
	assert(arg);
	dirty_area_widget *r = (dirty_area_widget *)arg;
	assert(r != 0);
	AM_DBG ambulant::lib::logger::get_logger()->debug("gtk_C_callback_helper_queue_draw_area with left: %d, top: %d, width: %d, height: %d s_widgets=%d", r->area.left(), r->area.top(), r->area.width(), r->area.height(),gtk_ambulant_widget::s_widgets);
	gtk_ambulant_widget::s_lock.enter();
	if (gtk_ambulant_widget::s_widgets > 0)
		gtk_widget_queue_draw_area(r->widget,
			r->area.left(), r->area.top(), r->area.width(), r->area.height());

	AM_DBG ambulant::lib::logger::get_logger()->debug("gtk_C_callback_helper_queue_draw_area with left: %d, top: %d, width: %d, height: %d tag=%d", r->area.left(), r->area.top(), r->area.width(), r->area.height(),r->tag);

	r->ambulant_widget->m_draw_area_tags.erase(r->tag);
	gtk_ambulant_widget::s_lock.leave();
//	gtk_widget_queue_draw(r->widget);
	delete r;
	return false;
//	r->release();
}
}


extern "C" {
void gtk_C_callback_do_paint_event(void *userdata, GdkEventExpose *event, GtkWidget *widget)
{
	((gtk_ambulant_widget*) userdata)->do_paint_event(event);
}
}

extern "C" {
void gtk_C_callback_do_motion_notify_event(void *userdata, GdkEventMotion *event, GtkWidget *widget)
{
	((gtk_ambulant_widget*) userdata)->do_motion_notify_event(event);
}
}

extern "C" {
void gtk_C_callback_do_button_release_event(void *userdata, GdkEventButton *event, GtkWidget *widget)
{
	gint s_x, s_y, d_x, d_y;
	gtk_ambulant_widget* gaw = (gtk_ambulant_widget*) userdata;
	GdkWindow* s_gdkw; /* source window */
	GtkWidget* d_gtkw; /* destination widget */
	GtkWidget* t_gtkw; /* toplevel widget */

	// Find in the widget stack the widget in which window
	// the event source coordinates (s_x,s_y) are defined
	
	if (gaw == NULL || event == NULL)
		return; /* cannot handle */
	s_x = (gint) round (event->x);
	s_y = (gint) round (event->y);
	s_gdkw = event->window;

	d_gtkw = gaw->get_gtk_widget ();
	t_gtkw = gtk_widget_get_toplevel (d_gtkw);

	for (GtkWidget* a_gtkw = d_gtkw;
		a_gtkw != NULL;
		a_gtkw = gtk_widget_get_parent (a_gtkw))
	{
		if (s_gdkw == a_gtkw->window) {
			/* found corresponding GdkWindow in GtkWidget stack */
			/* translate if necessary */
			if (a_gtkw != d_gtkw
				&& gtk_widget_translate_coordinates (a_gtkw, d_gtkw, s_x, s_y, &d_x, &d_y))
			{
				event->x = d_x;
				event->y = d_y;
			}
			gaw->do_button_release_event (event);
			break;
		}
		if (a_gtkw == t_gtkw)
			break; /* not found */
	}
}
}//extern "C"


extern "C" {
void gtk_C_callback_do_key_release_event(void *userdata, GdkEventKey *event, GtkWidget *widget)
{
	gtk_ambulant_widget* gaw = (gtk_ambulant_widget*) userdata;

	// Find in the widget stack the widget in which window
	// the event source coordinates (s_x,s_y) are defined
	
	if (gaw == NULL || event == NULL)
		return; /* cannot handle */
	gaw ->do_key_release_event (event);
}
}//extern "C"

void gui::gtk::gdk_pixmap_bitblt(
	GdkPixmap* dst, int dst_x, int dst_y,
	GdkPixmap* src, int src_x, int src_y,
	int width, int height)
{
	/*AM_DBG*/lib::logger::get_logger()->debug("gdk_pixmap_bitblt() s=(%d,%d),d=(%d,%d),wh=(%d,%d))", src_x, src_y, dst_x, dst_y, width, height);
#ifdef WITH_GTK3
	cairo_t *cr = gdk_cairo_create (GDK_DRAWABLE(dst));
	cairo_rectangle(cr, dst_x, dst_y, width, height);
	cairo_clip (cr);
	gdk_cairo_set_source_pixmap (cr, src, dst_x, dst_y);
	cairo_paint (cr);
	cairo_destroy (cr);
#else
	GdkGC *gc = gdk_gc_new (dst);
	gdk_draw_pixmap(GDK_DRAWABLE(dst), gc, GDK_DRAWABLE(src), src_x, src_y, dst_x, dst_y, width, height);
	g_object_unref (G_OBJECT (gc));
#endif//WITH_GTK3
};


//
// gtk_window_factory
//

gtk_window_factory::gtk_window_factory( gtk_ambulant_widget* parent_widget, common::gui_player* gpl)
:	m_gui_player(gpl),
	m_parent_widget(parent_widget)
{
	AM_DBG lib::logger::get_logger()->debug("gtk_window_factory (0x%x)", (void*) this);
	m_arrow_cursor = gdk_cursor_new(GDK_ARROW);
	m_hand1_cursor = gdk_cursor_new(GDK_HAND1);
	m_hand2_cursor = gdk_cursor_new(GDK_HAND2);
}

gtk_window_factory::~gtk_window_factory( )
{
	gdk_cursor_unref(m_arrow_cursor);
	gdk_cursor_unref(m_hand1_cursor);
	gdk_cursor_unref(m_hand2_cursor);
}

common::gui_window *
gtk_window_factory::new_window (const std::string &name, lib::size bounds, common::gui_events *region)
{
	lib::rect r (m_p, bounds);
	AM_DBG lib::logger::get_logger()->debug("gtk_window_factory::new_window (0x%x): name=%s %d,%d,%d,%d", (void*) this, name.c_str(), r.left(),r.top(),r.right(),r.bottom());
	ambulant_gtk_window * agtkw = new ambulant_gtk_window(name, &r, region);


	// We don't create this widget anymore MainLoop does it!!
	gtk_widget_set_size_request(m_parent_widget->get_gtk_widget(), r.right(), r.bottom());
	gtk_widget_set_size_request(
		gtk_widget_get_toplevel (m_parent_widget->get_gtk_widget()),
		r.right(),
		r.bottom()+ 25);
	gtk_widget_show(m_parent_widget->get_gtk_widget());
//	gtk_ambulant_widget * gtkaw = new gtk_ambulant_widget(name, r, m_parent_widget);

	// Wrong!!! I need to add the GUI size
//	gtk_widget_set_size_request(GTK_WIDGET (gtk_widget_get_toplevel (gtkaw->get_gtk_widget())), r.right(), r.bottom()+ 25);
//	agtkw->set_ambulant_widget(gtkaw);
	agtkw->set_ambulant_widget(m_parent_widget);
	m_parent_widget->set_gtk_window(agtkw);
//	gtkaw->set_gtk_window(agtkw);
	AM_DBG lib::logger::get_logger()->debug("gtk_window_factory::new_window(0x%x): ambulant_widget=0x%x gtk_window=0x%x", (void*) this, (void*) m_parent_widget, (void*) agtkw);
	agtkw->set_gui_player(m_gui_player);
	agtkw->set_gdk_cursor(GDK_ARROW, m_arrow_cursor);
	agtkw->set_gdk_cursor(GDK_HAND1, m_hand1_cursor);
	agtkw->set_gdk_cursor(GDK_HAND2, m_hand2_cursor);
	return agtkw;
}

common::bgrenderer *
gtk_window_factory::new_background_renderer(const common::region_info *src)
{
	AM_DBG lib::logger::get_logger()->debug("gtk_window_factory::new_background_renderer(0x%x): src=0x%x", (void*) this, src);
	return new gtk_background_renderer(src);
}



//
// ambulant_gtk_window
//

ambulant_gtk_window::ambulant_gtk_window(const std::string &name,
	lib::rect* bounds,
	common::gui_events *region)
:	common::gui_window(region),
	m_bounds(*bounds),
	m_ambulant_widget(NULL),
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
	m_surface(NULL)
{
	m_pixmap = NULL;
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::ambulant_gtk_window(0x%x)",(void *)this);
}

ambulant_gtk_window::~ambulant_gtk_window()
{

	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::~ambulant_gtk_window(0x%x): m_ambulant_widget=0x%x, m_pixmap=0x%x",this,m_ambulant_widget, m_pixmap);
	// Note that we don't destroy the widget, only sver the connection.
	// the widget itself is destroyed independently.
	if (m_ambulant_widget ) {
		m_ambulant_widget->set_gtk_window(NULL);
//XXXX next delete Reload crashes with gtk, not with qt
//		delete m_ambulant_widget;
		m_ambulant_widget = NULL;
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
ambulant_gtk_window::set_gdk_cursor(GdkCursorType gdk_cursor_type, GdkCursor* gdk_cursor)
{
	switch (gdk_cursor_type) {
	case GDK_ARROW: m_arrow_cursor = gdk_cursor;
	case GDK_HAND1: m_hand1_cursor = gdk_cursor;
	case GDK_HAND2: m_hand2_cursor = gdk_cursor;
	default:	return;
	}

}

GdkCursor*
ambulant_gtk_window::get_gdk_cursor(GdkCursorType gdk_cursor_type)
{
	switch (gdk_cursor_type) {
	case GDK_ARROW: return m_arrow_cursor;
	case GDK_HAND1: return m_hand1_cursor;
	case GDK_HAND2: return m_hand2_cursor;
	default:	return NULL;
	}
}

void
ambulant_gtk_window::need_redraw(const lib::rect &r)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::need_redraw(0x%x): ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.width(), r.height());
	if (m_ambulant_widget == NULL) {
		lib::logger::get_logger()->error("ambulant_gtk_window::need_redraw(0x%x): m_ambulant_widget == NULL !!!", (void*) this);
		return;
	}
	// we use the parent window for redraw in case this window has been deleted at the time
	// the callback function is actually called (e.g. the user selects a different file)
	GtkWidget* this_widget = m_ambulant_widget->get_gtk_widget();
	dirty_area_widget* dirty = new dirty_area_widget();
	dirty->widget = (gtk_ambulant_widget*) gtk_widget_get_parent(this_widget);
	dirty->area = r;
	if ( ! gtk_widget_translate_coordinates (this_widget, dirty->widget, r.left(), r.top(), &dirty->area.x, &dirty->area.y)) {
		AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::need_redraw(0x%x): gtk_widget_translate_coordinates failed.", (void *)this);
	}
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::need_redraw: parent ltrb=(%d,%d,%d,%d)", dirty->area.left(), dirty->area.top(), dirty->area.width(), dirty->area.height());
	dirty->ambulant_widget = m_ambulant_widget;
	gtk_ambulant_widget::s_lock.enter();
	guint draw_area_tag = g_timeout_add_full(G_PRIORITY_DEFAULT, 1, (GSourceFunc) gtk_C_callback_helper_queue_draw_area, (void *)dirty, NULL);
	dirty->tag = draw_area_tag;
	dirty->ambulant_widget->m_draw_area_tags.insert(draw_area_tag);
	gtk_ambulant_widget::s_lock.leave();
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::need_redraw: parent ltrb=(%d,%d,%d,%d), tag=%d fun=0x%x", dirty->area.left(), dirty->area.top(), dirty->area.width(), dirty->area.height(), draw_area_tag, gtk_C_callback_helper_queue_draw_area);
}

void
ambulant_gtk_window::redraw(const lib::rect &r)
{

	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::redraw(0x%x): ltrb=(%d,%d,%d,%d)",(void *)this, r.left(), r.top(), r.width(), r.height());
	_screenTransitionPreRedraw();
	clear();
	m_handler->redraw(r, this);
//XXXX	if ( ! isEqualToPrevious(m_pixmap))
	_screenTransitionPostRedraw(r);
	gdk_pixmap_bitblt(
		m_ambulant_widget->get_gtk_widget()->window, r.left(), r.top(),
		m_pixmap, r.left(), r.top(),
		r.width(), r.height());
	DUMPPIXMAP(m_pixmap, "pxmp");
#ifdef WITH_SCREENSHOTS
	GError *error = NULL;
	gint width; gint height;

#ifdef WITH_GTK3
	width = gdk_window_get_width (m_ambulant_widget->get_gtk_widget()->window);
	height = gdk_window_get_height(m_ambulant_widget->get_gtk_widget()->window);
#else
	gdk_drawable_get_size(m_ambulant_widget->get_gtk_widget()->window, &width, &height);
#endif//WITH_GTK3
	GdkPixbuf* pixbuf = gdk_pixbuf_get_from_drawable(
		NULL,
		m_ambulant_widget->get_gtk_widget()->window,
		0, 0, 0, 0, 0, width, height);
//	if (!gdk_pixbuf_save_to_buffer (pixbuf, &buffer, &buffer_size, "jpeg", &error, "quality", "100", NULL)) {
	if (m_ambulant_widget->m_screenshot_data) {
		g_free(m_ambulant_widget->m_screenshot_data);
		m_ambulant_widget->m_screenshot_data = NULL;
		m_ambulant_widget->m_screenshot_size = 0;
	}

	if (!gdk_pixbuf_save_to_buffer (
		pixbuf,
		&m_ambulant_widget->m_screenshot_data,
		&m_ambulant_widget->m_screenshot_size,
		"jpeg", &error, "quality", "100", NULL))
	{
		printf (" Tenemos un error%s", error->message);
		g_error_free (error);
	}
	DUMPPIXBUF(pixbuf, "pxbf");
	g_object_unref (G_OBJECT (pixbuf));
#endif //WITH_SCREENSHOTS
}

void
ambulant_gtk_window::redraw_now()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::redraw_now()");
}

bool
ambulant_gtk_window::user_event(const lib::point &where, int what)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::user_event(0x%x): point=(%d,%d)", this, where.x, where.y);
	return m_handler->user_event(where, what);
}

void
ambulant_gtk_window::need_events(bool want)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::need_events(0x%x): want=%d", this, want);
}

void
ambulant_gtk_window::set_ambulant_widget(gtk_ambulant_widget* gtkaw)
{

	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::set_ambulant_widget(0x%x)",(void *)gtkaw);
	// Don't destroy!
	//if (m_ambulant_widget != NULL)
	//	delete m_ambulant_widget;
	m_ambulant_widget = gtkaw;
	GdkColor color;
	GdkColormap *cmap = gdk_colormap_get_system();

	if (gtkaw != NULL) {

		// color is white
		gdk_color_parse("white", &color);

		// in debugging mode, initialize with purple background
		AM_DBG gdk_color_parse("Purple", &color);
		gdk_colormap_alloc_color(cmap, &color, FALSE, TRUE);

		// set the color in the widget
		gtk_widget_modify_bg (GTK_WIDGET (gtkaw->get_gtk_widget()), GTK_STATE_NORMAL, &color );

		// Initialize m_pixmap
		gint width; gint height;
		gtk_widget_get_size_request(GTK_WIDGET (gtkaw->get_gtk_widget()), &width, &height);
		m_pixmap = gdk_pixmap_new(gtkaw->get_gtk_widget()->window, width, height, -1);
		AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::set_ambulant_widget(0x%x); size (%i,%i)",(void *)gtkaw, width, height);
		// User Interaction
	}
//	}else{
//		gtk_widget_hide(GTK_WIDGET (gtkaw->get_gtk_widget()));
//		gtk_box_pack_start (GTK_BOX (parent_widget), GTK_WIDGET (m_widget), TRUE, TRUE, 0);
//		gtk_container_remove(GTK_CONTAINER (gtkaw->get_gtk_widget()->parent), GTK_WIDGET (gtkaw->get_gtk_widget()));
//		free(gtkaw->get_gtk_widget());
//	}
}

gtk_ambulant_widget*
ambulant_gtk_window::get_ambulant_widget()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::get_ambulant_widget(0x%x)",(void *)m_ambulant_widget);
	return m_ambulant_widget;
}

GdkPixmap*
ambulant_gtk_window::get_ambulant_pixmap()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::ambulant_pixmap(0x%x) = 0x%x",(void *)this,(void *)m_pixmap);
	return m_pixmap;
}

void
ambulant_gtk_window::set_gui_player(gui_player* gpl)
{
	m_gui_player = gpl;
}

gui_player*
ambulant_gtk_window::get_gui_player()
{
	return m_gui_player;
}

GdkPixmap*
ambulant_gtk_window::new_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::new_ambulant_surface(0x%x)",(void *)m_surface);
	if (m_surface != NULL) {
		delete m_surface;
		m_surface = NULL;
	}
#ifdef WITH_GTK3
	// TBD
#else
	gint width; gint height;
	gdk_drawable_get_size(GDK_DRAWABLE (m_pixmap), &width, &height);
	m_surface = gdk_pixmap_new(m_pixmap, width, height, -1);
#endif// WITH_GTK3
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::new_ambulant_surface(0x%x)",(void *)m_surface);
	return m_surface;
}

GdkPixmap*
ambulant_gtk_window::get_ambulant_oldpixmap()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::get_ambulant_oldpixmap(0x%x) = 0x%x",(void *)this,(void *)m_oldpixmap);
	if (m_fullscreen_count && m_fullscreen_old_pixmap)
		return m_fullscreen_old_pixmap;
	return m_oldpixmap;
}

GdkPixmap*
ambulant_gtk_window::get_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::get_ambulant_surface(0x%x) = 0x%x",(void *)this,(void *)m_surface);
	return m_surface;
}

GdkPixmap*
ambulant_gtk_window::get_pixmap_from_screen(const lib::rect &r)
{
	GdkPixmap *rv = gdk_pixmap_new(m_ambulant_widget->get_gtk_widget()->window, r.width(), r.height(), -1);
	gdk_pixmap_bitblt(rv, r.left(), r.top(), m_pixmap, r.left(), r.top(), r.width(), r.height());
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::get_pixmap_from_screen(0x%x) = 0x%x",(void *)this,(void *)m_pixmap);
	return rv;
}

void
ambulant_gtk_window::reset_ambulant_surface(void)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::reset_ambulant_surface(0x%x) m_oldpixmap = 0x%x",(void *)this,(void *)m_oldpixmap);
	if (m_oldpixmap != NULL) m_pixmap = m_oldpixmap;
}

void
ambulant_gtk_window::set_ambulant_surface(GdkPixmap* surf)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::set_ambulant_surface(0x%x) surf = 0x%x",(void *)this,(void *)surf);
	m_oldpixmap = m_pixmap;
	if (surf != NULL) m_pixmap = surf;
}

void
ambulant_gtk_window::delete_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::delete_ambulant_surface(0x%x) m_surface = 0x%x",(void *)this, (void *)m_surface);
	delete m_surface;
	m_surface = NULL;
}

// Transitions

void
ambulant_gtk_window::startScreenTransition()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::startScreenTransition()");
	if (m_fullscreen_count)
		logger::get_logger()->trace("%s:multiple Screen transitions in progress (m_fullscreen_count=%d)","ambulant_gtk_window::startScreenTransition()",m_fullscreen_count);
	m_fullscreen_count++;
	if (m_fullscreen_old_pixmap) g_object_unref(G_OBJECT(m_fullscreen_old_pixmap));
	m_fullscreen_old_pixmap = m_fullscreen_prev_pixmap;
	m_fullscreen_prev_pixmap = NULL;
}

void
ambulant_gtk_window::endScreenTransition()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::endScreenTransition()");
	assert(m_fullscreen_count > 0);
	m_fullscreen_count--;
}

void
ambulant_gtk_window::screenTransitionStep(smil2::transition_engine* engine, lib::transition_info::time_type now)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::screenTransitionStep()");
	assert(m_fullscreen_count > 0);
	m_fullscreen_engine = engine;
	m_fullscreen_now = now;
}

void
ambulant_gtk_window::_screenTransitionPreRedraw()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::_screenTransitionPreRedraw()");
	if (m_fullscreen_count == 0) return;
	// XXX setup drawing to transition surface
}

void
ambulant_gtk_window::_screenTransitionPostRedraw(const lib::rect &r)
{

	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::_screenTransitionPostRedraw()");
	if (m_fullscreen_count == 0 && m_fullscreen_old_pixmap == NULL) {
		// Neither in fullscreen transition nor wrapping one up.
		// Take a snapshot of the screen and return.
		AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::_screenTransitionPostRedraw: screen snapshot");
		if (m_fullscreen_prev_pixmap) g_object_unref(G_OBJECT(m_fullscreen_prev_pixmap));

		m_fullscreen_prev_pixmap = get_pixmap_from_screen(r); // XXX wrong
//		DUMPPIXMAP(m_fullscreen_prev_pixmap, "snap");
		return;
	}
	if (m_fullscreen_old_pixmap == NULL) {
		// Just starting a new fullscreen transition. Get the
		//	bits from the snapshot saved during the previous
		// redraw.
		m_fullscreen_old_pixmap = m_fullscreen_prev_pixmap;
		m_fullscreen_prev_pixmap = NULL;
	}

	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::_screenTransitionPostRedraw: bitblit");
	if (m_fullscreen_engine) {
		// Do the transition step
		GdkPixmap* new_src = get_ambulant_surface();
		if ( ! new_src) new_src = new_ambulant_surface();
		gdk_pixmap_bitblt(m_surface, 0, 0, m_pixmap, r.left(), r.top(), r.width(), r.height());
		gdk_pixmap_bitblt(m_pixmap, 0, 0, m_fullscreen_old_pixmap, r.left(), r.top(), r.width(), r.height());
		DUMPPIXMAP(new_src, "fnew");
		DUMPPIXMAP(m_pixmap, "fold");
		m_fullscreen_engine->step(m_fullscreen_now);
		DUMPPIXMAP(m_pixmap, "fres");
	}

	if (m_fullscreen_count == 0) {
		// Finishing a fullscreen transition.
		AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::_screenTransitionPostRedraw: cleanup after transition done");
		if (m_fullscreen_old_pixmap) g_object_unref(G_OBJECT(m_fullscreen_old_pixmap));
		m_fullscreen_old_pixmap = NULL;
		m_fullscreen_engine = NULL;
	}
}

void
ambulant_gtk_window::clear()
// private helper: clear the widget
{
	// Fill with <brush> color
	if (m_pixmap == NULL) {
		AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::clear(): m_pixmap == NULL!!");
		return;
	}
	color_t color = lib::to_color(255, 255, 255);
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::clear(): clearing to 0x%x", (long)color);
	GdkColor bgc;
	bgc.red = redc(color)*0x101;
	bgc.blue = bluec(color)*0x101;
	bgc.green = greenc(color)*0x101;
#ifdef WITH_GTK3
	// TBD
#else
	GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (m_pixmap));
	gdk_gc_set_rgb_fg_color (gc, &bgc);
	gdk_draw_rectangle (GDK_DRAWABLE (m_pixmap), gc, TRUE,
		m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h);
	g_object_unref (G_OBJECT (gc));
#endif//WITH_GTK3
}


//
// gtk_ambulant_widget
//
// gtk_ambulant_widget::s_widgets is a counter to check for the liveliness of gtk_widget during
// execution of gtk*draw() functions by a callback function in the main thread
// gtk_ambulant_widget::s_lock is for the protection of the counter
// TBD: a better approach would be to have s static protected std::vector<dirty_widget>
// to be updated when callbacks are scheduled and executed
// and use these entries to remove any scheduled callbacks with
// gboolean g_idle_remove_by_data (gpointer data); when the gtk_widget is destroyed
// then the ugly dependence on the parent widget couls also be removed
int gtk_ambulant_widget::s_widgets = 0;
lib::critical_section gtk_ambulant_widget::s_lock;

gtk_ambulant_widget::gtk_ambulant_widget(GtkWidget* widget)
:	m_gtk_window(NULL),
	m_screenshot_data(NULL),
	m_screenshot_size(0)
{
	m_widget = widget;
	GObject* ancestor_widget = G_OBJECT (GTK_WIDGET (gtk_widget_get_ancestor(m_widget, GTK_TYPE_WIDGET)));

	AM_DBG lib::logger::get_logger()->debug("gtk_ambulant_widget::gtk_ambulant_widget(0x%x-0x%x) s_widgets=%d",
		(void *)this,
		(void*) widget, gtk_ambulant_widget::s_widgets);
	m_expose_event_handler_id = g_signal_connect_swapped (G_OBJECT (m_widget),
		"expose_event", G_CALLBACK (gtk_C_callback_do_paint_event), (void*) this);
	m_motion_notify_handler_id = g_signal_connect_swapped (ancestor_widget,
		"motion_notify_event", G_CALLBACK (gtk_C_callback_do_motion_notify_event), (void*) this);
	m_button_release_handler_id = g_signal_connect_swapped (ancestor_widget,
		"button_release_event", G_CALLBACK (gtk_C_callback_do_button_release_event), (void*) this);
	m_key_release_handler_id = g_signal_connect_swapped (ancestor_widget,
		"key_release_event", G_CALLBACK (gtk_C_callback_do_key_release_event), (void*) this);
	lib::logger::get_logger()->debug("gtk_ambulant_widget::gtk_ambulant_widget(0x%x-0x%x) m_key_release_handler_id=%0x%x", this, widget, m_key_release_handler_id);
	gtk_widget_add_events(GTK_WIDGET (ancestor_widget),
		GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
	// widget needs focus for receiving key press/release events
	
#ifdef WITH_GTK3
	// TBD
#else
	GTK_WIDGET_SET_FLAGS(ancestor_widget, GTK_CAN_FOCUS);
#endif//WITH_GTK3
	gtk_widget_grab_focus(GTK_WIDGET(ancestor_widget));
	gtk_ambulant_widget::s_lock.enter();
	gtk_ambulant_widget::s_widgets++;
	gtk_ambulant_widget::s_lock.leave();
}

gtk_ambulant_widget::~gtk_ambulant_widget()
{
	gtk_ambulant_widget::s_lock.enter();
	gtk_ambulant_widget::s_widgets--;
	if ( ! m_draw_area_tags.empty()) {
		for (std::set<guint>::iterator it = m_draw_area_tags.begin(); it != m_draw_area_tags.end(); it++) {
			AM_DBG ambulant::lib::logger::get_logger()->debug("gtk_ambulant_widget::~gtk_ambulant_widget removing tag %d", (*it));
			g_source_remove((*it));

		}
	}
	gtk_ambulant_widget::s_lock.leave();
	AM_DBG lib::logger::get_logger()->debug("gtk_ambulant_widget::~gtk_ambulant_widget(0x%x): m_gtk_window=0x%x s_widgets=%d", (void*)this, m_gtk_window, gtk_ambulant_widget::s_widgets);
	GObject* ancestor_widget = G_OBJECT (GTK_WIDGET (gtk_widget_get_ancestor(m_widget, GTK_TYPE_WIDGET)));
	if (g_signal_handler_is_connected (G_OBJECT (m_widget), m_expose_event_handler_id))
		g_signal_handler_disconnect(G_OBJECT (m_widget), m_expose_event_handler_id);
	if (g_signal_handler_is_connected (ancestor_widget, m_motion_notify_handler_id))
		g_signal_handler_disconnect(ancestor_widget, m_motion_notify_handler_id);
	if (g_signal_handler_is_connected (ancestor_widget, m_button_release_handler_id))
		g_signal_handler_disconnect(ancestor_widget, m_button_release_handler_id);
	if (g_signal_handler_is_connected (ancestor_widget, m_key_release_handler_id))
		g_signal_handler_disconnect(ancestor_widget, m_key_release_handler_id);
	if (m_gtk_window) {
		m_gtk_window->set_ambulant_widget(NULL);
		m_gtk_window = NULL;
	}
	if (m_screenshot_data) {
		g_free(m_screenshot_data);
		m_screenshot_data = NULL;
		m_screenshot_size = 0;
	}
}

void
gtk_ambulant_widget::set_gtk_window( ambulant_gtk_window* agtkw)
{
	// Note: the window and widget are destucted independently.
	//	if (m_gtk_window != NULL)
	//	  delete m_gtk_window;
	m_gtk_window = agtkw;
	AM_DBG lib::logger::get_logger()->debug("gtk_ambulant_widget::set_gtk_window(0x%x): m_gtk_window==0x%x)",
		(void*) this, (void*) m_gtk_window);
}

ambulant_gtk_window*
gtk_ambulant_widget::gtk_window() {
	return m_gtk_window;
}

GtkWidget*
gtk_ambulant_widget::get_gtk_widget()
{
	return m_widget;
}

void gtk_ambulant_widget::do_paint_event (GdkEventExpose *e) {
	AM_DBG lib::logger::get_logger()->debug("gtk_ambulant_widget::paintEvent(0x%x): e=0x%x)", (void*) this, (void*) e);
	lib::rect r = lib::rect(
		lib::point(e->area.x, e->area.y),
		lib::size(e->area.width, e->area.height));
	if (m_gtk_window == NULL) {
		lib::logger::get_logger()->debug("gtk_ambulant_widget::paintEvent(0x%x): e=0x%x m_gtk_window==NULL",
			(void*) this, (void*) e);
		return;
	}
	m_gtk_window->redraw(r);
}

void
gtk_ambulant_widget::do_motion_notify_event(GdkEventMotion *e) {
	int m_o_x = 0, m_o_y = 0; //27; // XXXX Origin of MainWidget
	AM_DBG lib::logger::get_logger()->debug("gtk_ambulant_widget::mouseMoveEvent(0x%x) e=(%ld,%ld) m_gtk_window=0x%x\n", this, e->x,e->y, m_gtk_window);
	if (! m_gtk_window) return;
	//XXXX This is not right!!!
	ambulant::lib::point ap = ambulant::lib::point((int)e->x, (int)e->y);
	gui_player* gui_player =  m_gtk_window->get_gui_player();
	if (gui_player) {
		gui_player->before_mousemove(0);
		m_gtk_window->user_event(ap, 1);
	}
	int cursid = 0;
	if (gui_player)
		cursid = gui_player->after_mousemove();

	// Set hand cursor if cursid==1, arrow if cursid==0.
	GdkCursor* cursor;
	// gdk cursors need to be cached by the window factory
	cursor = cursid == 0 
		? m_gtk_window->get_gdk_cursor(GDK_ARROW)
		: m_gtk_window->get_gdk_cursor(GDK_HAND1);
	if (cursor)
		gdk_window_set_cursor (m_widget->window, cursor);
}

void
gtk_ambulant_widget::do_button_release_event(GdkEventButton *e) {
	AM_DBG lib::logger::get_logger()->debug("gtk_ambulant_widget::do_button_release_event(0x%x): e=0x%x, position=(%d, %d))", (void*) this, (void*) e, e->x, e->y);
	if (m_gtk_window == NULL) {
		lib::logger::get_logger()->debug("gtk_ambulant_widget::do_button_release_event(0x%x): e=0x%x  position=(%d, %d) m_gtk_window==NULL", (void*) this, (void*) e, e->x, e->y);
		return;
	}
	if (e->type == GDK_BUTTON_RELEASE){
		lib::point amwhere = lib::point((int)e->x, (int)e->y);
		m_gtk_window->user_event(amwhere);
	}
}

void
gtk_ambulant_widget::do_key_release_event(GdkEventKey *e) {
	AM_DBG lib::logger::get_logger()->debug("gtk_ambulant_widget::do_key_release_event(0x%x): e=0x%x  key=%d, length=%d string=%s) m_gtk_window==NULL", (void*) this, (void*) e, e->keyval, e->length, e->string);
	if (m_gtk_window == NULL) {
		lib::logger::get_logger()->debug("gtk_ambulant_widget::do_key_release_event(0x%x): e=0x%x  key=%d, length=%d string=%s) m_gtk_window==NULL", (void*) this, (void*) e, e->keyval, e->length, e->string);
		return;
	}
	if (e->type == GDK_KEY_RELEASE){
		lib::logger::get_logger()->debug("gtk_ambulant_widget::do_key_release_event(0x%x): e=0x%x  key=%d, length=%d string=%s) m_gtk_window==NULL", (void*) this, (void*) e, e->keyval, e->length, e->string);
//		m_gtk_window->user_event(amwhere);
		m_gtk_window->get_gui_player()->on_char((int) *e->string);
	}
}

void gtk_ambulant_widget::get_size(int *width, int *height){
#if 0
	gdk_drawable_get_size(m_widget->window, width, height);
#else
#endif
}

bool gtk_ambulant_widget::get_screenshot(const char *type, char **out_data, size_t *out_size){
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
bool gtk_ambulant_widget::set_overlay(const char *type, const char *data, size_t size){
	return FALSE;
}

// Not implemented
bool gtk_ambulant_widget::clear_overlay(){
	return FALSE;
}


bool gtk_ambulant_widget::set_screenshot(char **screenshot_data, size_t *screenshot_size){
	return TRUE;
}
