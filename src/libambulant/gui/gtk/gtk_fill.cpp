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

#include "ambulant/gui/gtk/gtk_includes.h"
#include "ambulant/gui/gtk/gtk_renderer.h"
#include "ambulant/gui/gtk/gtk_factory.h"
#include "ambulant/gui/gtk/gtk_fill.h"
#include "ambulant/gui/gtk/gtk_transition.h"
#include "ambulant/gui/gtk/gtk_image_renderer.h"
#include "ambulant/gui/gtk/gtk_text_renderer.h"
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
	lib::color_t bgcolor = info ? info->get_bgcolor() : lib::rrggbb_to_color(0xffffff);
	AM_DBG lib::logger::get_logger()->debug("gtk_fill_renderer.redraw_body: clearing to 0x%x", (long)color);
#ifdef WITH_GTK3
//	cairo_surface_t* pm = agtkw->get_ambulant_surface();
	cairo_surface_t* pm = agtkw->get_target_surface();
	GdkRGBA bgc;
	bgc.red = redf(color);
	bgc.blue = bluef(color);
	bgc.green = greenf(color);
	bgc.alpha = 1.0;
#else
	GdkColor bgc;
	bgc.red = redc(color)*0x101;
	bgc.blue = bluec(color)*0x101;
	bgc.green = greenc(color)*0x101;
	GdkPixmap* pm = agtkw->get_ambulant_surface();
#endif//WITH_GTK3
	if (pm == NULL) {
		pm = agtkw->get_ambulant_pixmap();
	}
#ifdef WITH_GTK3
	cairo_t* cr = cairo_create(agtkw->get_ambulant_pixmap());
	gdk_cairo_set_source_rgba (cr, &bgc);
	cairo_rectangle (cr, L, T, W, H);
	cairo_fill(cr);
	cairo_destroy(cr);
#else
	GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (pm));
	gdk_gc_set_rgb_fg_color (gc, &bgc);
	gdk_draw_rectangle (GDK_DRAWABLE (pm), gc, TRUE, L, T, W, H);
	g_object_unref (G_OBJECT (gc));
#endif//WITH_GTK3
	AM_DBG lib::logger::get_logger()->debug("gtk_fill_renderer.redraw_body(0x%x, local_ltrb=(%d,%d,%d,%d)",(void *)this, L,T,W,H);
}

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
#ifdef WITH_GTK3
		GdkRGBA bgc;
		bgc.alpha = 1.0;
#else
		GdkColor bgc;
#endif//WITH_GTK3
		bgc.red = redf(bgcolor);
		bgc.blue = bluef(bgcolor);
		bgc.green = greenf(bgcolor);
		if (opacity == 1.0) {
#ifdef WITH_GTK3
			cairo_t* cr = cairo_create(agtkw->get_target_surface());
//			cairo_t* cr = cairo_create(agtkw->get_ambulant_pixmap());
//			cairo_set_source_rgba (cr, bgc.red, bgc.green, bgc.blue, bgc.alpha);
			cairo_set_source_rgb (cr, bgc.red, bgc.green, bgc.blue);
			cairo_rectangle (cr, L, T, W, H);
			cairo_fill(cr);
			cairo_destroy(cr);
#else
			GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()));
			gdk_gc_set_rgb_fg_color (gc, &bgc);
			gdk_draw_rectangle (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()), gc, TRUE, L, T, W, H);
			g_object_unref (G_OBJECT (gc));
#endif//WITH_GTK3
		} else {  //XXXX adapted from gtk_transition. May be some code to be factored out
			// Method:
			// 1. Get the current on-screen image as a pixmap
			// 2. Create a new pixmap and draw a coloured rectangle on it
			// 3. Blend these 2 pixmaps together by getting their pixbufs
			// 4. Draw the resulting pixbuf to become the new on-screen image
			gint width; gint height;
#ifdef WITH_GTK3
			cairo_surface_t* opm = agtkw->get_ambulant_pixmap();
			width = cairo_image_surface_get_width (opm);
			height = cairo_image_surface_get_height (opm);
//X			cairo_content_t content_type = cairo_surface_get_content (opm);
			cairo_surface_t* npm = cairo_surface_create_similar_image (opm, CAIRO_FORMAT_ARGB32, width, height);
//X			gdk_pixmap_get_size(GDK_DRAWABLE (opm), &width, &height);
//X			cairo_surface_t* npm = gdk_pixmap_new(opm, width, height, -1);
			GdkPixbuf* old_pixbuf = gdk_pixbuf_get_from_surface (opm, L, T, width, height);
			GdkPixbuf* new_pixbuf = gdk_pixbuf_get_from_surface (npm, L, T, width, height);
#else
			GdkPixmap* opm = agtkw->get_ambulant_pixmap();
			gdk_pixmap_get_size(GDK_DRAWABLE (opm), &width, &height);
			GdkPixmap* npm = gdk_pixmap_new(opm, width, height, -1);
			GdkPixbuf* old_pixbuf = gdk_pixbuf_get_from_drawable(NULL, opm, NULL, L, T, 0, 0, W, H);
			GdkPixbuf* new_pixbuf = gdk_pixbuf_get_from_drawable(NULL, npm, NULL, L, T, 0, 0, W, H);
#endif//WITH_GTK3
#ifdef WITH_GTK3
			// TBD
#else
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
#endif//WITH_GTK3
		}
		//gtk_widget_modify_bg (GTK_WIDGET (agtkw->get_ambulant_widget()->get_gtk_widget()), GTK_STATE_NORMAL, &bgc );
#ifdef WITH_GTK3
		if (m_background_surface) {
#else
		if (m_background_pixmap) {
#endif//WITH_GTK3
			AM_DBG lib::logger::get_logger()->debug("gtk_background_renderer::redraw: drawing pixmap");
		//	paint.drawPixmap(L, T, *m_background_pixmap);
		}
	}
}

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
#ifdef WITH_GTK3
	if (m_background_surface) {
		cairo_surface_destroy ( m_background_surface);
		m_background_surface = NULL;
	}
	m_background_surface = agtkw->get_pixmap_from_screen(dstrect_whole);
#else
	if (m_background_pixmap) {
		delete m_background_pixmap;
		m_background_pixmap = NULL;
	}
	m_background_pixmap = agtkw->get_pixmap_from_screen(dstrect_whole);
#endif//WITH_GTK3
//XXXX	dumpPixmap(m_background_pixmap, "/tmp/keepbg");
}
