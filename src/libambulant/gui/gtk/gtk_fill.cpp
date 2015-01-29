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

#include "ambulant/gui/gtk/gtk_renderer.h"
#include "ambulant/gui/gtk/gtk_factory.h"
#include "ambulant/gui/gtk/gtk_fill.h"
#include "ambulant/gui/gtk/gtk_transition.h"
#include "ambulant/gui/gtk/gtk_image_renderer.h"
#include "ambulant/gui/gtk/gtk_text_renderer.h"
#include "ambulant/gui/gtk/gtk_util.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace gui::gtk;

extern const char gtk_fill_playable_tag[] = "brush";
extern const char gtk_fill_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererGtk");
extern const char gtk_fill_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererFill");

common::playable_factory *
gui::gtk::create_gtk_fill_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererGtk"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererFill"), true);
	return new common::single_playable_factory<
		gtk_fill_renderer,
		gtk_fill_playable_tag,
		gtk_fill_playable_renderer_uri,
		gtk_fill_playable_renderer_uri2,
		gtk_fill_playable_renderer_uri2>(factory, mdp);
}

#if GTK_MAJOR_VERSION >= 3
void
gtk_fill_renderer::redraw_body(const lib::rect &dirty, common::gui_window *window) {

	const common::region_info *info = m_dest->get_info();
	const lib::rect &r = m_dest->get_rect();
	ambulant_gtk_window* agtkw = (ambulant_gtk_window*) window;
	// <brush> drawing
	// First find our whole area to be cleared to <brush> color
	lib::rect dstrect_whole = r;
	dstrect_whole.translate(m_dest->get_global_topleft());
	int L = dstrect_whole.left(),
		T = dstrect_whole.top(),
		W = dstrect_whole.width(),
		H = dstrect_whole.height();
	// Fill with  color
	const char *color_attr = m_node->get_attribute("color");
	if (!color_attr) {
		lib::logger::get_logger()->trace("<brush> element without color attribute");
		return;
	}
	// Fill with <brush> color
	color_t color = lib::to_color(color_attr);
	AM_DBG lib::logger::get_logger()->debug("gtk_fill_renderer.redraw_body: clearing to 0x%x", (long)color);
	cairo_surface_t* pm = agtkw->get_target_surface();
	GdkRGBA gcol;
	gcol.red = redf(color);
	gcol.blue = bluef(color);
	gcol.green = greenf(color);
	gcol.alpha = 1.0;
	cairo_t* cr = cairo_create(agtkw->get_target_surface());
	gdk_cairo_set_source_rgba (cr, &gcol);
	cairo_rectangle (cr, L, T, W, H);
	cairo_fill(cr);
	cairo_destroy(cr);
	AM_DBG lib::logger::get_logger()->debug("gtk_fill_renderer.redraw_body(0x%x, local_ltrb=(%d,%d,%d,%d)",(void *)this, L,T,W,H);
}
#else // GTK_MAJOR_VERSION < 3
void
gtk_fill_renderer::redraw_body(const lib::rect &dirty, common::gui_window *window) {

	const common::region_info *info = m_dest->get_info();
	const lib::rect &r = m_dest->get_rect();
	ambulant_gtk_window* agtkw = (ambulant_gtk_window*) window;
	// <brush> drawing
	// First find our whole area to be cleared to <brush> color
	lib::rect dstrect_whole = r;
	dstrect_whole.translate(m_dest->get_global_topleft());
	int L = dstrect_whole.left(),
		T = dstrect_whole.top(),
		W = dstrect_whole.width(),
		H = dstrect_whole.height();
	// Fill with  color
	const char *color_attr = m_node->get_attribute("color");
	if (!color_attr) {
		lib::logger::get_logger()->trace("<brush> element without color attribute");
		return;
	}
	// Fill with <brush> color
	color_t color = lib::to_color(color_attr);
	AM_DBG lib::logger::get_logger()->debug("gtk_fill_renderer.redraw_body: clearing to 0x%x", (long)color);
	// Fill with <brush> color
	GdkColor gcol;
	gcol.red = redc(color)*0x101;
	gcol.blue = bluec(color)*0x101;
	gcol.green = greenc(color)*0x101;
	GdkPixmap* pm = agtkw->get_ambulant_surface();
	if (pm == NULL) {
		pm = agtkw->get_ambulant_pixmap();
	}
	GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (pm));
	gdk_gc_set_rgb_fg_color (gc, &gcol);
	gdk_draw_rectangle (GDK_DRAWABLE (pm), gc, TRUE, L, T, W, H);
	g_object_unref (G_OBJECT (gc));
	AM_DBG lib::logger::get_logger()->debug("gtk_fill_renderer.redraw_body(0x%x, local_ltrb=(%d,%d,%d,%d)",(void *)this, L,T,W,H);
}
#endif // GTK_MAJOR_VERSION < 3

void
gtk_fill_renderer::start(double where)
{
    if (m_transition_renderer != NULL) {
        m_transition_renderer->start(where);
    }
    if (m_context != NULL) {
        m_context->started(m_cookie);
        m_context->stopped(m_cookie);
    }
    renderer_playable::start(where);
}

#if GTK_MAJOR_VERSION >= 3
void
gtk_background_renderer::redraw(const lib::rect &dirty, common::gui_window *window)
{
	if ( !	(m_src && m_dst))
		return;
	const lib::rect &r = m_dst->get_rect();
	AM_DBG lib::logger::get_logger()->debug("gtk_background_renderer::redraw(0x%x)", (void *)this);
	ambulant_gtk_window* agtkw = (ambulant_gtk_window*) window;
	cairo_t* cr = cairo_create(agtkw->get_target_surface());
	// First find our whole area to be cleared to background color
	lib::rect dstrect_whole = r;
	dstrect_whole.translate(m_dst->get_global_topleft());
	int	L = dstrect_whole.left(),
		T = dstrect_whole.top(),
		W = dstrect_whole.width(),
		H = dstrect_whole.height();
		// Fill with background color
		lib::color_t bgcolor = m_src->get_bgcolor();
		AM_DBG lib::logger::get_logger()->debug("gtk_background_renderer::redraw: clearing to %x, agtkw=0x%x local_ltwh(%d,%d,%d,%d)",(long)bgcolor,(void*)agtkw,L,T,W,H);
	double opacity = m_src->get_bgopacity();
	if (opacity > 0.0) {
		GdkRGBA bgc;
		bgc.alpha = 1.0;
		bgc.red = redf(bgcolor);
		bgc.blue = bluef(bgcolor);
		bgc.green = greenf(bgcolor);

		cairo_set_source_rgba (cr, bgc.red, bgc.green, bgc.blue, opacity);
		cairo_rectangle (cr, L, T, W, H);
		cairo_fill(cr);
	}
	std::string id = m_dst->get_info()->get_name();
	AM_DBG logger::get_logger()->debug("%s: m_dst=0x%x", __PRETTY_FUNCTION__, id.c_str());
	cairo_destroy(cr);
}
#else // GTK_MAJOR_VERSION < 3
void
gtk_background_renderer::redraw(const lib::rect &dirty, common::gui_window *window)
{
	if ( !	(m_src && m_dst))
		return;
	const lib::rect &r = m_dst->get_rect();
	AM_DBG lib::logger::get_logger()->debug("gtk_background_renderer::redraw(0x%x)", (void *)this);
	double opacity = m_src->get_bgopacity();
	if (opacity > 0.0) {
	// First find our whole area to be cleared to background color
		ambulant_gtk_window* agtkw = (ambulant_gtk_window*) window;
		lib::rect dstrect_whole = r;
		dstrect_whole.translate(m_dst->get_global_topleft());
		int L = dstrect_whole.left(),
			T = dstrect_whole.top(),
			W = dstrect_whole.width(),
			H = dstrect_whole.height();
		// XXXX Fill with background color
		lib::color_t bgcolor = m_src->get_bgcolor();
		AM_DBG lib::logger::get_logger()->debug("gtk_background_renderer::redraw: clearing to %x, agtkw=0x%x local_ltwh(%d,%d,%d,%d)",(long)bgcolor,(void*)agtkw,L,T,W,H);
		GdkColor bgc;
		bgc.red = redc(bgcolor)*0x101;
		bgc.blue = bluec(bgcolor)*0x101;
		bgc.green = greenc(bgcolor)*0x101;
		if (opacity == 1.0) {
			GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()));
			gdk_gc_set_rgb_fg_color (gc, &bgc);
			gdk_draw_rectangle (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()), gc, TRUE, L, T, W, H);
			DUMPPIXMAP(agtkw->get_ambulant_pixmap(), "bkgd");
			g_object_unref (G_OBJECT (gc));

		} else {  //XXXX adapted from gtk_transition. May be some code to be factored out
			// Method:
			// 1. Get the current on-screen image as a pixmap
			// 2. Create a new pixmap and draw a coloured rectangle on it
			// 3. Blend these 2 pixmaps together by getting their pixbufs
			// 4. Draw the resulting pixbuf to become the new on-screen image
			gint width; gint height;
			GdkPixmap* opm = agtkw->get_ambulant_pixmap();
			gdk_pixmap_get_size(GDK_DRAWABLE (opm), &width, &height);
			GdkPixmap* npm = gdk_pixmap_new(opm, width, height, -1);
			GdkPixbuf* old_pixbuf = gdk_pixbuf_get_from_drawable(NULL, opm, NULL, L, T, 0, 0, W, H);
			GdkPixbuf* new_pixbuf = gdk_pixbuf_get_from_drawable(NULL, npm, NULL, L, T, 0, 0, W, H);
			GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (opm));
			GdkGC *ngc = gdk_gc_new (GDK_DRAWABLE (npm));
			gdk_gc_set_rgb_fg_color (ngc, &bgc);
			gdk_draw_rectangle (GDK_DRAWABLE (npm), ngc, TRUE, L, T, W, H);
			int alpha = static_cast<int>(round(255*opacity));
			gdk_pixbuf_composite(new_pixbuf, old_pixbuf,0,0,W,H,0,0,1,1,GDK_INTERP_BILINEAR, alpha);
			gdk_draw_pixbuf(opm, gc, old_pixbuf, 0, 0, L, T, W, H, GDK_RGB_DITHER_NONE,0,0);
			g_object_unref (G_OBJECT (old_pixbuf));
			g_object_unref (G_OBJECT (new_pixbuf));
			g_object_unref (G_OBJECT (ngc));
			g_object_unref (G_OBJECT (gc));
		}
	}
}
#endif // GTK_MAJOR_VERSION < 3

void gtk_background_renderer::highlight(gui_window *window)
{
}

void
gtk_background_renderer::keep_as_background()
{
	AM_DBG lib::logger::get_logger()->debug("gtk_background_renderer::keep_as_background(0x%x) called", (void *)this);
	const lib::rect &r = m_dst->get_rect();
	ambulant_gtk_window* agtkw = (ambulant_gtk_window*) m_dst->get_gui_window();
	lib::rect dstrect_whole = r;
	dstrect_whole.translate(m_dst->get_global_topleft());
#if GTK_MAJOR_VERSION >= 3
	if (m_background_surface) {
		cairo_surface_destroy ( m_background_surface);
		m_background_surface = NULL;
	}
	m_background_surface = agtkw->get_surface_from_screen(dstrect_whole);
#else // GTK_MAJOR_VERSION < 3
	if (m_background_pixmap) {
		delete m_background_pixmap;
		m_background_pixmap = NULL;
	}
	m_background_pixmap = agtkw->get_pixmap_from_screen(dstrect_whole);
#endif // GTK_MAJOR_VERSION < 3
}
