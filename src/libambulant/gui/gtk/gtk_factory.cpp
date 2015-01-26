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

//#define AM_DBG if(1)
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include "ambulant/gui/gtk/gtk_factory.h"
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

#if GTK_MAJOR_VERSION >= 3
extern "C" {
void gtk_C_callback_do_draw_event(void *userdata, cairo_t *cr, GtkWidget *widget)
{
	((gtk_ambulant_widget*) userdata)->do_draw_event(widget, cr);
}
}
#else // GTK_MAJOR_VERSION < 3
extern "C" {
void gtk_C_callback_do_paint_event(void *userdata, GdkEventExpose *event, GtkWidget *widget)
{
	((gtk_ambulant_widget*) userdata)->do_paint_event(event);
}
}
#endif // GTK_MAJOR_VERSION < 3

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
	        if (s_gdkw == gtk_widget_get_window (a_gtkw)) {
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

#if GTK_MAJOR_VERSION >= 3
void gui::gtk::cairo_surface_bitblt(
	cairo_surface_t* dst, int dst_x, int dst_y,
	cairo_surface_t* src, int src_x, int src_y,
	int width, int height)
{
	AM_DBG lib::logger::get_logger()->debug("cairo_surface_t() s=(%d,%d),d=(%d,%d),wh=(%d,%d))", src_x, src_y, dst_x, dst_y, width, height);
	cairo_t *cr = cairo_create (dst);
	cairo_rectangle(cr, dst_x, dst_y, width, height);
	cairo_set_source_surface (cr, src, 0,0);
	cairo_fill (cr);
	cairo_destroy (cr);
};
#else // GTK_MAJOR_VERSION < 3
void gui::gtk::gdk_pixmap_bitblt(
	GdkPixmap* dst, int dst_x, int dst_y,
	GdkPixmap* src, int src_x, int src_y,
	int width, int height)
{
	AM_DBG lib::logger::get_logger()->debug("gdk_pixmap_bitblt() s=(%d,%d),d=(%d,%d),wh=(%d,%d))", src_x, src_y, dst_x, dst_y, width, height);
	GdkGC *gc = gdk_gc_new (dst);
	gdk_draw_pixmap(GDK_DRAWABLE(dst), gc, GDK_DRAWABLE(src), src_x, src_y, dst_x, dst_y, width, height);
	g_object_unref (G_OBJECT (gc));
};
#endif // GTK_MAJOR_VERSION < 3

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

#if GTK_MAJOR_VERSION >= 3

gtk_window_factory::~gtk_window_factory( )
{
	g_object_unref(m_arrow_cursor);
	g_object_unref(m_hand1_cursor);
	g_object_unref(m_hand2_cursor);
}
#else // GTK_MAJOR_VERSION < 3

gtk_window_factory::~gtk_window_factory( )
{
	gdk_cursor_unref(m_arrow_cursor);
	gdk_cursor_unref(m_hand1_cursor);
	gdk_cursor_unref(m_hand2_cursor);
}
#endif // GTK_MAJOR_VERSION < 3

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

#if GTK_MAJOR_VERSION >= 3
ambulant_gtk_window::ambulant_gtk_window(const std::string &name,
	lib::rect* bounds,
	common::gui_events *region)
:	common::gui_window(region),
	m_bounds(*bounds),
	m_ambulant_widget(NULL),
	m_gui_player(NULL),
	m_arrow_cursor(NULL),
	m_hand1_cursor(NULL),
	m_hand2_cursor(NULL),
	m_fullscreen_count(0),
	m_fullscreen_engine(NULL),
	m_fullscreen_now(0),
	m_target_surface(NULL),
	m_transition_surface(NULL),
	m_old_target_surface(NULL),
	m_fullscreen_prev_surface(NULL),
	m_fullscreen_old_surface(NULL),
	m_tmp_surface(NULL),
	m_surface(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::ambulant_gtk_window(0x%x)",(void *)this);
}
#else // GTK_MAJOR_VERSION < 3
ambulant_gtk_window::ambulant_gtk_window(const std::string &name,
	lib::rect* bounds,
	common::gui_events *region)
:	common::gui_window(region),
	m_bounds(*bounds),
	m_ambulant_widget(NULL),
	m_gui_player(NULL),
	m_arrow_cursor(NULL),
	m_hand1_cursor(NULL),
	m_hand2_cursor(NULL),
	m_fullscreen_count(0),
	m_fullscreen_engine(NULL),
	m_fullscreen_now(0),
	m_pixmap(NULL),
	m_tmppixmap(NULL),
	m_oldpixmap(NULL),
	m_fullscreen_prev_pixmap(NULL),
	m_fullscreen_old_pixmap(NULL),
	m_surface(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::ambulant_gtk_window(0x%x)",(void *)this);
}
#endif // GTK_MAJOR_VERSION < 3

#if GTK_MAJOR_VERSION >= 3
ambulant_gtk_window::~ambulant_gtk_window()
{

	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::~ambulant_gtk_window(0x%x): m_ambulant_widget=0x%x",this,m_ambulant_widget);
	// Note that we don't destroy the widget, only sver the connection.
	// the widget itself is destroyed independently.
	if (m_ambulant_widget ) {
		if (m_ambulant_widget->m_screenshot_data != NULL) {
			g_free(m_ambulant_widget->m_screenshot_data);
		}
		m_ambulant_widget->set_gtk_window(NULL);
//XXXX next delete Reload crashes with gtk, not with qt
//		delete m_ambulant_widget;
		m_ambulant_widget = NULL;
	}
	if (m_target_surface != NULL) {
		cairo_surface_destroy(m_target_surface);
	}
	if (m_surface != NULL) {
		cairo_surface_destroy(m_surface);
	}
	if (m_fullscreen_prev_surface != NULL) {
		cairo_surface_destroy(m_fullscreen_prev_surface);
	}
	if (m_fullscreen_old_surface != NULL) {
		cairo_surface_destroy(m_fullscreen_old_surface);
	}
	if (m_transition_surface != NULL) {
		cairo_surface_destroy(m_transition_surface);
	}
	if (m_ambulant_widget != NULL && m_ambulant_widget->m_screenshot_data != NULL) {
		g_free(m_ambulant_widget->m_screenshot_data);
	}
}
#else // GTK_MAJOR_VERSION < 3
ambulant_gtk_window::~ambulant_gtk_window()
{

	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::~ambulant_gtk_window(0x%x): m_ambulant_widget=0x%x",this,m_ambulant_widget);
	// Note that we don't destroy the widget, only sver the connection.
	// the widget itself is destroyed independently.
	if (m_ambulant_widget ) {
		if (m_ambulant_widget->m_screenshot_data != NULL) {
			g_free(m_ambulant_widget->m_screenshot_data);
//		delete m_ambulant_widget;
		m_ambulant_widget = NULL;
		}
		m_ambulant_widget->set_gtk_window(NULL);
//XXXX next delete Reload crashes with gtk, not with qt
//		delete m_ambulant_widget;
		m_ambulant_widget = NULL;
	}
	if (m_pixmap != NULL) {
		g_object_unref(G_OBJECT(m_pixmap));
	}
/*	'm_oldpixmap' is a placeholder, not a managed object */
	if (m_surface != NULL) {
		g_object_unref(G_OBJECT(m_surface));
	}
	if (m_fullscreen_prev_pixmap != NULL) {
		g_object_unref(G_OBJECT(m_fullscreen_prev_pixmap));
	}
	if (m_fullscreen_old_pixmap != NULL) {
		g_object_unref(G_OBJECT(m_fullscreen_old_pixmap));
	}
}
#endif // GTK_MAJOR_VERSION < 3

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
	if (this_widget == NULL || dirty->widget == NULL) {
		return;
	}
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

#if GTK_MAJOR_VERSION >= 3
void
ambulant_gtk_window::redraw(const lib::rect &r)
{
	GError *error = NULL;
	gint width; gint height;

	if (m_ambulant_widget == NULL) {
		return; // not yet set
	}
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::redraw(0x%x): ltrb=(%d,%d,%d,%d)",(void *)this, r.left(), r.top(), r.width(), r.height());
	_screenTransitionPreRedraw();
	clear();
	m_handler->redraw(r, this);
	_screenTransitionPostRedraw(r);
#ifdef WITH_SCREENSHOTS
	width = gdk_window_get_width (gtk_widget_get_window (m_ambulant_widget->get_gtk_widget()));
	height = gdk_window_get_height(gtk_widget_get_window (m_ambulant_widget->get_gtk_widget()));
	cairo_surface_t* surf = get_target_surface();
	if (surf == NULL) {
		// target surface not yet known
		return;
	}
	guint W = cairo_image_surface_get_width (surf);
	guint H = cairo_image_surface_get_height (surf);
	GdkPixbuf* pixbuf = gdk_pixbuf_get_from_surface (surf, 0, 0, W, H);
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
	g_object_unref (G_OBJECT (pixbuf));
#endif //WITH_SCREENSHOTS
}
#else // GTK_MAJOR_VERSION < 3
void
ambulant_gtk_window::redraw(const lib::rect &r)
{
	GError *error = NULL;
	gint width; gint height;

	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::redraw(0x%x): ltrb=(%d,%d,%d,%d)",(void *)this, r.left(), r.top(), r.width(), r.height());
	_screenTransitionPreRedraw();
	clear();
	m_handler->redraw(r, this);
	_screenTransitionPostRedraw(r);
	gdk_pixmap_bitblt(
		gtk_widget_get_window (m_ambulant_widget->get_gtk_widget()), r.left(), r.top(),
		m_pixmap, r.left(), r.top(),
		r.width(), r.height());
	DUMPPIXMAP(m_pixmap, "pxmp");
	gdk_drawable_get_size(m_ambulant_widget->get_gtk_widget()->window, &width, &height);
	GdkPixbuf* pixbuf = gdk_pixbuf_get_from_drawable(
		NULL,
		gtk_widget_get_window (m_ambulant_widget->get_gtk_widget()),
		0, 0, 0, 0, 0, width, height);
#ifdef WITH_SCREENSHOTS
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
#endif //WITH_SCREENSHOTS
	g_object_unref (G_OBJECT (pixbuf));
}
#endif // GTK_MAJOR_VERSION < 3

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
#if GTK_MAJOR_VERSION >= 3

void
ambulant_gtk_window::set_ambulant_widget(gtk_ambulant_widget* gtkaw)
{

	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::set_ambulant_widget(0x%x)",(void *)gtkaw);
	// Don't destroy!
	//if (m_ambulant_widget != NULL)
	//	delete m_ambulant_widget;
	m_ambulant_widget = gtkaw;
	/* Initialize the surface to white */
	clear ();
}
#else // GTK_MAJOR_VERSION < 3

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
		m_pixmap = gdk_pixmap_new(gtk_widget_get_window (gtkaw->get_gtk_widget()), width, height, -1);
		AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::set_ambulant_widget(0x%x); size (%i,%i)",(void *)gtkaw, width, height);
		// User Interaction
	}
}
#endif // GTK_MAJOR_VERSION < 3

gtk_ambulant_widget*
ambulant_gtk_window::get_ambulant_widget()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::get_ambulant_widget(0x%x)",(void *)m_ambulant_widget);
	return m_ambulant_widget;
}

#if GTK_MAJOR_VERSION >= 3
	// TBD
#else // GTK_MAJOR_VERSION < 3
GdkPixmap*
ambulant_gtk_window::get_ambulant_pixmap()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::ambulant_pixmap(0x%x) = 0x%x",(void *)this,(void *)m_pixmap);
	return m_pixmap;
}
#endif // GTK_MAJOR_VERSION < 3

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
#if GTK_MAJOR_VERSION >= 3
cairo_surface_t*
ambulant_gtk_window::create_similar_surface(cairo_surface_t* surface) {
	cairo_surface_t* new_surface = NULL;
	if (surface != NULL && cairo_surface_get_type(surface) == CAIRO_SURFACE_TYPE_IMAGE) {
		int W = cairo_image_surface_get_width (surface);
		int H = cairo_image_surface_get_height (surface);
		new_surface = cairo_surface_create_similar_image(surface, CAIRO_FORMAT_ARGB32, W, H);
	  }
	  return new_surface;
}
#endif // GTK_MAJOR_VERSION >= 3

#if GTK_MAJOR_VERSION >= 3
cairo_surface_t*
ambulant_gtk_window::new_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::new_ambulant_surface(0x%x)",(void *)m_surface);
	if (m_surface != NULL) {
	// TBD
		cairo_surface_destroy (m_surface);
	}
	cairo_surface_t* target = get_target_surface();
	if (target != NULL) {
		m_surface = create_similar_surface(target);
	} else {
		m_surface = NULL;
	}
	return m_surface;
}
#else // GTK_MAJOR_VERSION < 3
GdkPixmap*
ambulant_gtk_window::new_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::new_ambulant_surface(0x%x)",(void *)m_surface);
	if (m_surface != NULL) {
		delete m_surface;
		m_surface = NULL;
	}
	gint width; gint height;
	gdk_drawable_get_size(GDK_DRAWABLE (m_pixmap), &width, &height);
	m_surface = gdk_pixmap_new(m_pixmap, width, height, -1);
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::new_ambulant_surface(0x%x)",(void *)m_surface);
	return m_surface;
}
#endif // GTK_MAJOR_VERSION < 3

#if GTK_MAJOR_VERSION >= 3
cairo_surface_t*
ambulant_gtk_window::get_old_target_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::get_old_target_surface(0x%x) = 0x%x",(void *)this,(void *)m_old_target_surface);
	if (m_fullscreen_count && m_fullscreen_old_surface)
		return m_fullscreen_old_surface;
	return m_old_target_surface;
}
#else // GTK_MAJOR_VERSION < 3
GdkPixmap*
ambulant_gtk_window::get_ambulant_oldpixmap()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::get_ambulant_oldpixmap(0x%x) = 0x%x",(void *)this,(void *)m_oldpixmap);
	if (m_fullscreen_count && m_fullscreen_old_pixmap)
		return m_fullscreen_old_pixmap;
	return m_oldpixmap;
}
#endif // GTK_MAJOR_VERSION < 3

#if GTK_MAJOR_VERSION >= 3
cairo_surface_t*
ambulant_gtk_window::get_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::get_ambulant_surface(0x%x) = 0x%x",(void *)this,(void *)m_surface);
	return m_surface;
}
#else // GTK_MAJOR_VERSION < 3
GdkPixmap*
ambulant_gtk_window::get_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::get_ambulant_surface(0x%x) = 0x%x",(void *)this,(void *)m_surface);
	return m_surface;
}
#endif // GTK_MAJOR_VERSION < 3


#if GTK_MAJOR_VERSION >= 3
cairo_surface_t*
ambulant_gtk_window::copy_surface(cairo_surface_t* srf, rect* rp)
{
	cairo_surface_t* rv = NULL;
	int L = 0, T = 0, W, H;

	if (srf != NULL) {
		if (rp == NULL) {
			W = cairo_image_surface_get_width (srf);
			H = cairo_image_surface_get_height (srf);
		} else {
			L = rp->left();
			T = rp->top();
			W = rp->width();
			H = rp->height();
		}

 		rv = cairo_surface_create_similar_image (srf, CAIRO_FORMAT_ARGB32, W, H);
		if (rv != NULL) {
			cairo_t* cr = cairo_create (rv);
			cairo_rectangle (cr, L, T, W, H);
			cairo_set_source_surface (cr, srf, -L, -T);
			cairo_paint (cr);
			cairo_destroy (cr);
		}
	}
	return rv;
}

cairo_surface_t*
ambulant_gtk_window::get_surface_from_screen(const lib::rect &r)
{
	cairo_surface_t* rv = copy_surface (m_target_surface);
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::get_surface_from_screen(0x%x) = 0x%x",(void *)this,(void *) rv);
	return rv;
}
#else // GTK_MAJOR_VERSION < 3
GdkPixmap*
ambulant_gtk_window::get_pixmap_from_screen(const lib::rect &r)
{
	GdkPixmap *rv = gdk_pixmap_new(m_ambulant_widget->get_gtk_widget()->window, r.width(), r.height(), -1);
	gdk_pixmap_bitblt(rv, r.left(), r.top(), m_pixmap, r.left(), r.top(), r.width(), r.height());
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::get_pixmap_from_screen(0x%x) = 0x%x",(void *)this,(void *) rv);
	return rv;
}
#endif // GTK_MAJOR_VERSION < 3

#if GTK_MAJOR_VERSION >= 3
void
ambulant_gtk_window::reset_target_surface(void)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::reset_ambulant_surface(%p) m_target_surface = %p, m_old_target_surface=%p", this, m_target_surface, m_old_target_surface);
	if (m_old_target_surface != NULL) m_target_surface = m_old_target_surface;
}

void
ambulant_gtk_window::set_target_surface(cairo_surface_t* surf)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::set_ambulant_surface(%p) surf = %p,  m_target_surface = %p, m_old_target_surface=%p", this, surf, m_target_surface, m_old_target_surface);
	if (surf != NULL && surf != m_target_surface) {
		m_old_target_surface = m_target_surface;
		m_target_surface = surf;
	}
}

#else // GTK_MAJOR_VERSION < 3

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
#endif // GTK_MAJOR_VERSION < 3

#if GTK_MAJOR_VERSION >= 3
void
ambulant_gtk_window::delete_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::delete_ambulant_surface(0x%x) m_surface = 0x%x",(void *)this, (void *)m_surface);
	if (m_surface != NULL) {
		cairo_surface_destroy (m_surface);
		m_surface = NULL;
	}
}
#else // GTK_MAJOR_VERSION < 3
void
ambulant_gtk_window::delete_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::delete_ambulant_surface(0x%x) m_surface = 0x%x",(void *)this, (void *)m_surface);
	delete m_surface;
	m_surface = NULL;
}
#endif // GTK_MAJOR_VERSION < 3

// Transitions
#if GTK_MAJOR_VERSION >= 3
cairo_surface_t*
ambulant_gtk_window::get_transition_surface()
{
	if (m_transition_surface == NULL) {
		m_transition_surface = create_similar_surface (m_target_surface);	     
	}
	return m_transition_surface;
}
#endif // GTK_MAJOR_VERSION >= 3

#if GTK_MAJOR_VERSION >= 3
void
ambulant_gtk_window::startScreenTransition()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::startScreenTransition()");
	if (m_fullscreen_count)
		logger::get_logger()->trace("%s:multiple Screen transitions in progress (m_fullscreen_count=%d)","ambulant_gtk_window::startScreenTransition()",m_fullscreen_count);
	m_fullscreen_count++;
	if (m_fullscreen_old_surface != NULL) {
		cairo_surface_destroy (m_fullscreen_old_surface);
		m_fullscreen_old_surface = NULL;
	}
	m_fullscreen_old_surface = m_fullscreen_prev_surface;
	m_fullscreen_prev_surface = NULL;
}
#else // GTK_MAJOR_VERSION < 3
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
#endif // GTK_MAJOR_VERSION < 3

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
#if GTK_MAJOR_VERSION >= 3

void
ambulant_gtk_window::_screenTransitionPostRedraw(const lib::rect &r)
{

	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::_screenTransitionPostRedraw()");
	if (m_fullscreen_count == 0 && m_fullscreen_old_surface == NULL) {
		// Neither in fullscreen transition nor wrapping one up.
		// Take a snapshot of the screen and return.
		AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::_screenTransitionPostRedraw: screen snapshot");
		if (m_fullscreen_prev_surface != NULL) {
			cairo_surface_destroy (m_fullscreen_prev_surface);
			m_fullscreen_prev_surface = NULL;
		}
		m_fullscreen_prev_surface = get_surface_from_screen(r); // XXX wrong
//		DUMPSURFACE(m_fullscreen_prev_surface, "snap");
		return;
	}
	if (m_fullscreen_old_surface == NULL) {
		// Just starting a new fullscreen transition. Get the
		//	bits from the snapshot saved during the previous
		// redraw.
		m_fullscreen_old_surface = m_fullscreen_prev_surface;
		m_fullscreen_prev_surface = NULL;
	}

	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::_screenTransitionPostRedraw: bitblit, m_fullscreen_old_surface=%p", m_fullscreen_old_surface);
	if (m_fullscreen_engine) {
		// Do the transition step
		cairo_surface_t* new_src = get_ambulant_surface();
		if (new_src == NULL) {
			new_src = new_ambulant_surface();
		}
		cairo_surface_bitblt(m_surface, 0, 0, m_target_surface, r.left(), r.top(), r.width(), r.height());
		cairo_surface_bitblt(m_target_surface, 0, 0, m_fullscreen_old_surface, r.left(), r.top(), r.width(), r.height());
		m_fullscreen_engine->step(m_fullscreen_now);
	}
	if (m_fullscreen_count == 0) {
		// Finishing a fullscreen transition.
		AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::_screenTransitionPostRedraw: cleanup after transition done");
		if (m_fullscreen_old_surface != NULL) {
			cairo_surface_destroy (m_fullscreen_old_surface);
			m_fullscreen_old_surface = NULL;
		}
		m_fullscreen_engine = NULL;
	}
}
#else // GTK_MAJOR_VERSION < 3

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

	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::_screenTransitionPostRedraw: bitblit, m_fullscreen_old_pixmap=%p", m_fullscreen_old_pixmap);
	if (m_fullscreen_engine) {
		// Do the transition step
		GdkPixmap* new_src = get_ambulant_surface();
		if ( ! new_src) new_src = new_ambulant_surface();
		gdk_pixmap_bitblt(m_surface, 0, 0, m_pixmap, r.left(), r.top(), r.width(), r.height());
		gdk_pixmap_bitblt(m_pixmap, 0, 0, m_fullscreen_old_pixmap, r.left(), r.top(), r.width(), r.height());
	}

	if (m_fullscreen_count == 0) {
		// Finishing a fullscreen transition.
		AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::_screenTransitionPostRedraw: cleanup after transition done");
		if (m_fullscreen_old_pixmap) g_object_unref(G_OBJECT(m_fullscreen_old_pixmap));
		m_fullscreen_old_pixmap = NULL;
		m_fullscreen_engine = NULL;
	}
}
#endif // GTK_MAJOR_VERSION < 3

#if GTK_MAJOR_VERSION >= 3
void
ambulant_gtk_window::clear()
// private helper: clear the widget
{
	// Fill with <brush> color
	color_t color = lib::to_color(255, 255, 255);
	GdkRGBA bgc;

	bgc.alpha = 1.0;
	bgc.red = redc(color)*0x101;
	bgc.blue = bluec(color)*0x101;
	bgc.green = greenc(color)*0x101;
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::clear(): clearing to 0x%x", (long) lib::to_color(bgc.red, bgc.green, bgc.blue));
	cairo_t *cr  = cairo_create (m_target_surface);
	cairo_set_source_rgba (cr, bgc.red, bgc.green, bgc.blue, bgc.alpha);
	cairo_paint (cr);
	cairo_destroy (cr);
}
#else // GTK_MAJOR_VERSION < 3
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
	bgc.green = greenc(color)*0x101;
	bgc.blue = bluec(color)*0x101;
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::clear(): clearing to 0x%x", (long) lib::to_color(bgc.red, bgc.green, bgc.blue));
	GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (m_pixmap));
	gdk_gc_set_rgb_fg_color (gc, &bgc);
	gdk_draw_rectangle (GDK_DRAWABLE (m_pixmap), gc, TRUE,
		m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.h);
	g_object_unref (G_OBJECT (gc));
}
#endif // GTK_MAJOR_VERSION < 3

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

#if GTK_MAJOR_VERSION >= 3
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
	
	m_expose_event_handler_id = g_signal_connect_swapped (m_widget,
	        "draw", (GCallback) (gtk_C_callback_do_draw_event), (void*) this);
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
	gtk_widget_set_can_focus (GTK_WIDGET(ancestor_widget), true);
	gtk_widget_grab_focus(GTK_WIDGET(ancestor_widget));
	gtk_ambulant_widget::s_lock.enter();
	gtk_ambulant_widget::s_widgets++;
	gtk_ambulant_widget::s_lock.leave();
}
#else // GTK_MAJOR_VERSION < 3
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
	GTK_WIDGET_SET_FLAGS(ancestor_widget, GTK_CAN_FOCUS);
	gtk_widget_grab_focus(GTK_WIDGET(ancestor_widget));
	gtk_ambulant_widget::s_lock.enter();
	gtk_ambulant_widget::s_widgets++;
	gtk_ambulant_widget::s_lock.leave();
}
#endif // GTK_MAJOR_VERSION < 3

gtk_ambulant_widget::~gtk_ambulant_widget()
{
	m_lock.enter();
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
	m_lock.leave();
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

#if GTK_MAJOR_VERSION >= 3
void
gtk_ambulant_widget::do_draw_event (GtkWidget *widget, cairo_t *cr) {
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("gtk_ambulant_widget::do_draw_event(%p): widget=%p cr=%p", this, widget, cr);
	if (cr == NULL || cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
		lib::logger::get_logger()->debug("gtk_ambulant_widget::do_draw_event(0x%p): wrong cairo status %s", (void*) this, cr == NULL ? "<null> ": cairo_status_to_string (cairo_status(cr)));
	}

	guint W = gtk_widget_get_allocated_width (widget);
	guint H = gtk_widget_get_allocated_height (widget);
	cairo_surface_t* target_surface = cairo_get_target (cr);
	static bool ls_type_known = false;
	static cairo_surface_type_t ls_surface_type = CAIRO_SURFACE_TYPE_IMAGE;
	if (ls_type_known == false) {
		ls_type_known = true;
		ls_surface_type = cairo_surface_get_type (target_surface);
		AM_DBG lib::logger::get_logger()->debug("gtk_ambulant_widget::do_draw_event(%p): surface_type=%d", this, ls_surface_type);
	}
	ambulant_gtk_window* agw = gtk_window();
	target_surface = agw->get_target_surface();
	if (target_surface == NULL || agw->get_target_bounds() !=  lib::rect(lib::point(0,0), lib::size(W, H))) {
		if (target_surface != NULL) {
			cairo_surface_destroy (target_surface);
		}
		target_surface = gdk_window_create_similar_image_surface
		  (gtk_widget_get_window (widget), CAIRO_FORMAT_ARGB32, W, H, 0);
		agw->set_target_bounds( lib::rect(lib::point(0,0), lib::size(W, H)));
		agw->set_drawing_surface (target_surface);
	}
	if (m_gtk_window == NULL) {
		lib::logger::get_logger()->debug("gtk_ambulant_widget::do_draw_event(0x%x):  m_gtk_window==NULL", (void*) this);
		m_lock.leave();
		return;
	}
	agw->redraw(agw->get_target_bounds());
	cairo_set_source_surface (cr, agw->get_target_surface(), 0, 0);
	cairo_paint (cr);
	m_lock.leave();
}
#else // GTK_MAJOR_VERSION < 3
void
gtk_ambulant_widget::do_paint_event (GdkEventExpose *e) {
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
#endif // GTK_MAJOR_VERSION < 3

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
		gdk_window_set_cursor (gtk_widget_get_window (m_widget), cursor);
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
#if GTK_MAJOR_VERSION >= 3
	*width = gdk_window_get_width (gtk_widget_get_window (m_widget));
	*height = gdk_window_get_height(gtk_widget_get_window (m_widget));
#else // GTK_MAJOR_VERSION < 3
	gdk_drawable_get_size(m_widget->window, width, height);
#endif // GTK_MAJOR_VERSION < 3
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
