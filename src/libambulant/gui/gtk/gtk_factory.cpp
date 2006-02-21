// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
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
#ifdef	WITH_QT_HTML_WIDGET
#include "ambulant/gui/gtk/gtk_html_renderer.h"
#endif/*WITH_QT_HTML_WIDGET*/
#include "ambulant/gui/gtk/gtk_text_renderer.h"
#include "ambulant/gui/gtk/gtk_video_renderer.h"


using namespace ambulant;
using namespace gui::gtk;
using namespace net;

// under construction

struct dirty_area_widget {
	lib::rect area;
	GtkWidget* widget;	
};

extern "C" {
bool gtk_C_callback_helper_queue_draw_area(void *arg)
{
	assert(arg);
	dirty_area_widget *r = (dirty_area_widget *)arg;
	assert(r != 0);
//	ambulant::lib::logger::get_logger()->debug("gtk_C_callback_helper_queue_draw_area with left: %d, top: %d, width: %d, height: %d", r->area.left(), r->area.top(), r->area.width(), r->area.height());
	gtk_widget_queue_draw_area(r->widget, r->area.left(), r->area.top(), r->area.width(), r->area.height());
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
	((gtk_ambulant_widget*) userdata)->do_button_release_event(event);
}
}


gtk_video_factory::~gtk_video_factory()
{
}

gtk_renderer_factory::gtk_renderer_factory(common::factories *factory)
:	m_factory(factory)
{
	AM_DBG lib::logger::get_logger()->debug("gtk_renderer factory (0x%x)", (void*) this);
}
	
gtk_window_factory::gtk_window_factory( GtkWidget* parent_widget, int x, int y, GMainLoop* loop)
//gtk_window_factory::gtk_window_factory( GtkWidget* parent_widget, int x, int y)
:	m_parent_widget(parent_widget), m_p(lib::point(x,y)) 
{
	m_main_loop = loop;
	AM_DBG lib::logger::get_logger()->debug("gtk_window_factory (0x%x)", (void*) this);
}	
  
ambulant_gtk_window::ambulant_gtk_window(const std::string &name,
	   lib::rect* bounds,
	   common::gui_events *region)
:	common::gui_window(region),
	m_ambulant_widget(NULL),
	m_oldpixmap(NULL),
	m_tmppixmap(NULL),
#ifdef USE_SMIL21
	m_fullscreen_count(0),
	m_fullscreen_prev_pixmap(NULL),
	m_fullscreen_old_pixmap(NULL),
	m_fullscreen_engine(NULL),
	m_fullscreen_now(0),
#endif
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
		delete m_ambulant_widget;
		m_ambulant_widget = NULL;
		free(m_pixmap);
		m_pixmap = NULL;
		if (m_tmppixmap != NULL) {
			free(m_tmppixmap);
			m_tmppixmap = NULL;
		}
	} 
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
		m_pixmap = gdk_pixmap_new(gtkaw->get_gtk_widget()->window,
                		width,
                          	height,
                          	-1);
		lib::logger::get_logger()->debug("ambulant_gtk_window::set_ambulant_widget(0x%x); size (%i,%i)",(void *)gtkaw, width, height);
		// User Interaction
	}		
//	}else{
//		gtk_widget_hide(GTK_WIDGET (gtkaw->get_gtk_widget()));
//		gtk_box_pack_start (GTK_BOX (parent_widget), GTK_WIDGET (m_widget), TRUE, TRUE, 0);
//		gtk_container_remove(GTK_CONTAINER (gtkaw->get_gtk_widget()->parent), GTK_WIDGET (gtkaw->get_gtk_widget()));
//		free(gtkaw->get_gtk_widget());
//	}
}

/*
ambulant::common::gui_screen* 
gtk_window::get_gui_screen(){
	return m_gui_screen;
}
*/

GdkPixmap* 
ambulant_gtk_window::get_ambulant_pixmap()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::ambulant_pixmap(0x%x) = 0x%x",(void *)this,(void *)m_pixmap);
	return m_pixmap;
}

GdkPixmap*
ambulant_gtk_window::get_pixmap_from_screen(const lib::rect &r)
{
	// This function does not work

	GdkPixmap *rv = gdk_pixmap_new(m_ambulant_widget->get_gtk_widget()->window,
                          	r.width(),
                          	r.height(),
                          	-1);
	GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (m_ambulant_widget->get_gtk_widget()));

	// draw the screen into m_pixmap
	gdk_draw_drawable(m_pixmap,
			gc, rv,
			r.left(), r.top(),
			r.left(), r.top(), 
			r.width(), r.height());
	g_object_unref (G_OBJECT (gc));
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::get_pixmap_from_screen(0x%x) = 0x%x",(void *)this,(void *)m_pixmap);
	return rv;
}

gtk_ambulant_widget*
ambulant_gtk_window::get_ambulant_widget()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::get_ambulant_widget(0x%x)",(void *)m_ambulant_widget);
	return m_ambulant_widget;
}

GdkPixmap*
ambulant_gtk_window::new_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::new_ambulant_surface(0x%x)",(void *)m_surface);
	if (m_surface != NULL) delete m_surface;
	gint width; gint height;
	gdk_drawable_get_size(GDK_DRAWABLE (m_pixmap), &width, &height);
	m_surface = gdk_pixmap_new(m_pixmap,
                          	width,
                          	height,
                          	-1);
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::new_ambulant_surface(0x%x)",(void *)m_surface);
        return m_surface;
}

GdkPixmap*
ambulant_gtk_window::get_ambulant_surface()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::get_ambulant_surface(0x%x) = 0x%x",(void *)this,(void *)m_surface);
        return m_surface;
}

GdkPixmap*
ambulant_gtk_window::get_ambulant_oldpixmap()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::get_ambulant_oldpixmap(0x%x) = 0x%x",(void *)this,(void *)m_oldpixmap);
#ifdef USE_SMIL21
	if (m_fullscreen_count && m_fullscreen_old_pixmap)
		return m_fullscreen_old_pixmap;
#endif
        return m_oldpixmap;
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

void
ambulant_gtk_window::need_redraw(const lib::rect &r)
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::need_redraw(0x%x): ltrb=(%d,%d,%d,%d)", (void *)this, r.left(), r.top(), r.width(), r.height());
	if (m_ambulant_widget == NULL) {
		lib::logger::get_logger()->error("ambulant_gtk_window::need_redraw(0x%x): m_ambulant_widget == NULL !!!", (void*) this);
		return;
	}

	dirty_area_widget* dirty = new dirty_area_widget();
	dirty->widget = m_ambulant_widget->get_gtk_widget();
	dirty->area = r;
	g_idle_add((GSourceFunc) gtk_C_callback_helper_queue_draw_area, (void *)dirty);

//	gtk_widget_queue_draw_area(m_ambulant_widget->get_gtk_widget(), r.left(), r.top(), r.width(), r.height());
//	gdk_threads_leave ();
//	g_main_context_dispatch(g_main_loop_get_context(m_main_loop)); 
}

void
ambulant_gtk_window::redraw_now()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::redraw_now()");
}

void
ambulant_gtk_window::mouse_region_changed()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::mouse_region_changed needs to be implemented");
}

/* test if there is something new to see */
static GdkPixmap* oldImageP;
static bool isEqualToPrevious(GdkPixmap* qpmP) {
	return false;
	//QImage img = qpmP->convertToImage();
	//if (oldImageP != NULL && img == *oldImageP) {
	//	AM_DBG lib::logger::get_logger()->debug("isEqualToPrevious: new image not different from old one");
	//	return true;
	//} else {
	//	if (oldImageP != NULL) delete oldImageP;
	//	oldImageP = new QImage(img);
	//	return false;
	//}
}

#ifdef DUMPPIXMAP
// doesn't compile on Zaurus
/**/
/* dumpPixmap on file */
void gui::gtk::dumpPixmap(GdkPixmap* qpm, std::string filename) {
/**
	GtkImage image;
	if ( ! qpm) return;
		gtk_image_set_from_pixmap (GTK_IMAGE (image), qpm, NULL);
//QImage img = qpm->convertToImage();
	if ( !
		char buf[5]; isEqualToPrevious(qpm)) {
		static int i;
		sprintf(buf,"%04d",i++);
		std::string newfile = buf + std::string(filename) +".png";
		qpm->save(newfile, "PNG");
		AM_DBG lib::logger::get_logger()->debug("dumpPixmap(%s)", newfile.c_str());
	}
**/
}
#endif

void
ambulant_gtk_window::redraw(const lib::rect &r)
{

	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::redraw(0x%x): ltrb=(%d,%d,%d,%d)",(void *)this, r.left(), r.top(), r.width(), r.height());
#ifdef USE_SMIL21
	_screenTransitionPreRedraw();
#endif
	m_handler->redraw(r, this);
//XXXX	if ( ! isEqualToPrevious(m_pixmap))
#ifdef USE_SMIL21
	_screenTransitionPostRedraw(r);
#endif
	GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (m_pixmap));
	gdk_draw_pixmap(m_ambulant_widget->get_gtk_widget()->window,
			gc,
			m_pixmap,
			r.left(), r.top(), 
			r.left(), r.top(),
			r.width(), r.height());
	

	// Get the screenshot of the last drawing
/*
	GError *error = NULL;
	gint width; gint height;
	gsize *buffer_size;
	gchar **buffer;
	gdk_drawable_get_size(m_ambulant_widget->get_gtk_widget()->window, &width, &height);
	GdkPixbuf* pixbuf = gdk_pixbuf_get_from_drawable(NULL, m_ambulant_widget->get_gtk_widget()->window, 0, 0, 0, 0, 0, width, height);
//	gdk_pixbuf_save(pixbuf, "myscreenshot.jpg", "jpeg", &error, "quality", "100", 0);
	gdk_pixbuf_save_to_buffer(pixbuf, buffer, buffer_size, "jpeg", &error, "quality", "100", 0);
*/
	g_object_unref (G_OBJECT (gc));
//XXXX	dumpPixmap(m_pixmap, "top"); //AM_DBG 
}

void
ambulant_gtk_window::user_event(const lib::point &where, int what) 
{
        AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::user_event(0x%x): point=(%d,%d)", this, where.x, where.y);
	m_handler->user_event(where, what);
}

void
ambulant_gtk_window::need_events(bool want) 
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::need_events(0x%x): want=%d", this, want);
}

// XXXX
gtk_ambulant_widget::gtk_ambulant_widget(const std::string &name,
	lib::rect* bounds,
	GtkWidget* parent_widget)
:	m_gtk_window(NULL)
{
//	m_widget = gtk_drawing_area_new();
	m_widget = parent_widget;
	AM_DBG lib::logger::get_logger()->debug("gtk_ambulant_widget::gtk_ambulant_widget(0x%x-0x%x(%d,%d,%d,%d))",
		(void *)this,
		(void*)  parent_widget,
		bounds->left(),
		bounds->top(),
		bounds->right(),
		bounds->bottom());

	gtk_widget_set_size_request(GTK_WIDGET (m_widget), bounds->right(), bounds->bottom());

//	gtk_box_pack_start (GTK_BOX (parent_widget), GTK_WIDGET (m_widget), TRUE, TRUE, 0);
	gtk_widget_show(m_widget);
	
	g_signal_connect_swapped (G_OBJECT (m_widget), "expose_event", G_CALLBACK (gtk_C_callback_do_paint_event), (void*) this);
	g_signal_connect_swapped (G_OBJECT (GTK_WIDGET (gtk_widget_get_toplevel(m_widget))), "motion_notify_event", G_CALLBACK (gtk_C_callback_do_motion_notify_event), (void*) this);
	g_signal_connect_swapped (G_OBJECT (GTK_WIDGET (gtk_widget_get_toplevel(m_widget))), "button_release_event", G_CALLBACK (gtk_C_callback_do_button_release_event), (void*) this);
	gtk_widget_add_events( (GTK_WIDGET (gtk_widget_get_toplevel(m_widget))), GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK);
}

gtk_ambulant_widget::~gtk_ambulant_widget()
{
	AM_DBG lib::logger::get_logger()->debug("gtk_ambulant_widget::~gtk_ambulant_widget(0x%x): m_gtk_window=0x%x", (void*)this, m_gtk_window);
	if (m_gtk_window) {
		m_gtk_window->set_ambulant_widget(NULL);
		m_gtk_window = NULL;
	}
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
gtk_ambulant_widget::do_button_release_event(GdkEventButton *e) {
	AM_DBG lib::logger::get_logger()->debug("gtk_ambulant_widget::do_button_release_event(0x%x): e=0x%x, position=(%d, %d))", (void*) this, (void*) e, e->x, e->y);
	if (m_gtk_window == NULL) {
		lib::logger::get_logger()->debug("gtk_ambulant_widget::do_button_release_event(0x%x): e=0x%x  position=(%d, %d) m_gtk_window==NULL", (void*) this, (void*) e, e->x, e->y);
		return;
	}
	if (e->type == GDK_BUTTON_RELEASE){
		lib::point amwhere = lib::point(e->x, e->y);
		m_gtk_window->user_event(amwhere);
	}
}

void 
gtk_ambulant_widget::do_motion_notify_event(GdkEventMotion *e) {
	int m_o_x = 0, m_o_y = 0; //27; // XXXX Origin of MainWidget
	AM_DBG lib::logger::get_logger()->debug("%s:(%d,%d)\n", "gtk_ambulant_widget::mouseMoveEvent", e->x,e->y);
	// This is not right!!!
	ambulant::lib::point ap = ambulant::lib::point(e->x,
						       e->y-25);
	m_gtk_window->user_event(ap, 1);
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

// XXXX
common::playable *
gtk_renderer_factory::new_playable(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp) 
{

	lib::xml_string tag = node->get_qname().second;
	common::playable* rv;
	if (tag == "img") {
 		rv = new gtk_image_renderer(context, cookie, node,
						  evp, m_factory);
		AM_DBG lib::logger::get_logger()->debug("gtk_renderer_factory: node 0x%x: returning gtk_image_renderer 0x%x", 
			(void*) node, (void*) rv);
	} else if (tag == "brush") {
 		rv = new gtk_fill_renderer(context, cookie, node,
					  evp, m_factory);
		AM_DBG lib::logger::get_logger()->debug("gtk_renderer_factory: node 0x%x: returning gtk_fill_renderer 0x%x", 
			(void*) node, (void*) rv);
	} else if ( tag == "text") {
#ifdef	WITH_GTK_HTML_WIDGET
		net::url url = net::url(node->get_url("src"));
		if (url.guesstype() == "text/html") {
			rv = new gtk_html_renderer(context, cookie, node, evp, m_factory);
			AM_DBG lib::logger::get_logger()->debug("gtk_renderer_factory: node 0x%x: returning gtk_html_renderer 0x%x", (void*) node, (void*) rv);
		} else {
#endif   /*WITH_GTK_HTML_WIDGET*/
		rv = new gtk_text_renderer(context, cookie, node,
						 evp, m_factory);
		AM_DBG lib::logger::get_logger()->debug("gtk_renderer_factory: node 0x%x: returning gtk_text_renderer 0x%x",
			(void*) node, (void*) rv);

#ifdef	WITH_GTK_HTML_WIDGET
		}
#endif/*WITH_QT_HTML_WIDGET*/
	} else {
		return NULL;
	}
  return rv;
}

common::playable *
gtk_renderer_factory::new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src)
{
	return NULL;
}
  
common::gui_window *
gtk_window_factory::new_window (const std::string &name,
			       lib::size bounds,
			       common::gui_events *region)
{
	lib::rect* r = new lib::rect(m_p, bounds);
	AM_DBG lib::logger::get_logger()->debug("gtk_window_factory::new_window (0x%x): name=%s %d,%d,%d,%d",
		(void*) this, name.c_str(), r->left(),r->top(),r->right(),r->bottom());
	ambulant_gtk_window * agtkw = new ambulant_gtk_window(name, r, region);
	agtkw->m_main_loop = m_main_loop;
	gtk_ambulant_widget * gtkaw = new gtk_ambulant_widget(name, r, m_parent_widget);
	
	// Wrong!!! I need to add the GUI size
	gtk_widget_set_size_request(GTK_WIDGET (gtk_widget_get_toplevel (gtkaw->get_gtk_widget())), r->right(), r->bottom()+ 25);
	agtkw->set_ambulant_widget(gtkaw);
	
	gtkaw->set_gtk_window(agtkw);
	AM_DBG lib::logger::get_logger()->debug("gtk_window_factory::new_window(0x%x): ambulant_widget=0x%x gtk_window=0x%x",
		(void*) this, (void*) gtkaw, (void*) agtkw);
	return agtkw;
}

common::bgrenderer *
gtk_window_factory::new_background_renderer(const common::region_info 
					   *src)
{
	AM_DBG lib::logger::get_logger()->debug("gtk_window_factory::new_background_renderer(0x%x): src=0x%x",
		(void*) this, src);
	return new gtk_background_renderer(src);
}

common::playable *
gtk_video_factory::new_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp)
{
	common::playable *rv;
	
	lib::xml_string tag = node->get_qname().second;
    AM_DBG lib::logger::get_logger()->debug("gtk_video_factory: node 0x%x:   inspecting %s\n", (void *)node, tag.c_str());
	if ( tag == "video") {
	  rv = new gtk_video_renderer(context, cookie, node, evp, m_factory);
		/*AM_DBG*/ lib::logger::get_logger()->debug("gtk_video_factory: node 0x%x:  returning gtk_video_renderer 0x%x", (void *)node, (void *)rv);
	} else {
		/*AM_DBG*/ lib::logger::get_logger()->debug("gtk_video_factory: no renderer for tag \"%s\"", tag.c_str());
		return NULL;
	}
	return rv;
}

common::playable *
gtk_video_factory::new_aux_audio_playable(
		common::playable_notification *context,
		common::playable_notification::cookie_type cookie,
		const lib::node *node,
		lib::event_processor *evp,
		net::audio_datasource *src)
{
	return NULL;
}
  
#ifdef USE_SMIL21

void 
ambulant_gtk_window::startScreenTransition()
{
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::startScreenTransition()");
	if (m_fullscreen_count)
		logger::get_logger()->warn("ambulant_gtk_window::startScreenTransition():multiple Screen transitions in progress (m_fullscreen_count=%d)",m_fullscreen_count);
	m_fullscreen_count++;
	if (m_fullscreen_old_pixmap) delete m_fullscreen_old_pixmap;
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
//	[[self getTransitionSurface] lockFocus];
}

void 
ambulant_gtk_window::_screenTransitionPostRedraw(const lib::rect &r)
{

	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::_screenTransitionPostRedraw()");
	if (m_fullscreen_count == 0 && m_fullscreen_old_pixmap == NULL) {
		// Neither in fullscreen transition nor wrapping one up.
		// Take a snapshot of the screen and return.
		AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::_screenTransitionPostRedraw: screen snapshot");
		if (m_fullscreen_prev_pixmap) delete m_fullscreen_prev_pixmap;
//		m_fullscreen_prev_pixmap = get_pixmap_from_screen(r); // XXX wrong
//		dumpPixmap(m_fullscreen_prev_pixmap, "snap");
		return;
	}
	if (m_fullscreen_old_pixmap == NULL) {
		// Just starting a new fullscreen transition. Get the
		//  bits from the snapshot saved during the previous
		// redraw.
		m_fullscreen_old_pixmap = m_fullscreen_prev_pixmap;
		m_fullscreen_prev_pixmap = NULL;
	}
	
	AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::_screenTransitionPostRedraw: bitblit");
	if (m_fullscreen_engine) {
		// Do the transition step
		GdkPixmap* new_src = get_ambulant_surface();
		if ( ! new_src) new_src = new_ambulant_surface();
//		bitBlt(m_surface, 0, 0, m_pixmap);
//		bitBlt(m_pixmap, 0, 0, m_fullscreen_old_pixmap);
//		dumpPixmap(new_src, "fnew");
//		dumpPixmap(m_pixmap, "fold");
		m_fullscreen_engine->step(m_fullscreen_now);
//		dumpPixmap(m_pixmap, "fres");
	}

	if (m_fullscreen_count == 0) {
		// Finishing a fullscreen transition.
		AM_DBG lib::logger::get_logger()->debug("ambulant_gtk_window::_screenTransitionPostRedraw: cleanup after transition done");
		if (m_fullscreen_old_pixmap) delete m_fullscreen_old_pixmap;
		m_fullscreen_old_pixmap = NULL;
		m_fullscreen_engine = NULL;
	}
}
#endif
