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

gtk_fill_renderer::~gtk_fill_renderer()
{
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("~gtk_fill_renderer(0x%x)", (void *)this);
	m_lock.leave();
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
	GdkColor bgc;
	bgc.red = redc(color)*0x101;
	bgc.blue = bluec(color)*0x101;
	bgc.green = greenc(color)*0x101;
	GdkPixmap* pm = agtkw->get_ambulant_surface();
	if (pm == NULL) {
		pm = agtkw->get_ambulant_pixmap();
	}
	GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (pm));
	gdk_gc_set_rgb_fg_color (gc, &bgc);
	gdk_draw_rectangle (GDK_DRAWABLE (pm), gc, TRUE, L, T, W, H);
	g_object_unref (G_OBJECT (gc));
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
		GdkColor bgc;
		bgc.red = redc(bgcolor)*0x101;
		bgc.blue = bluec(bgcolor)*0x101;
		bgc.green = greenc(bgcolor)*0x101;
		if (opacity == 1.0) {
			GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()));
			gdk_gc_set_rgb_fg_color (gc, &bgc);
			gdk_draw_rectangle (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()), gc, TRUE, L, T, W, H);
			g_object_unref (G_OBJECT (gc));
		} else {  //XXXX adapted from gtk_transition. May be some code to be factored out
			// Method:
			// 1. Get the current on-screen image as a pixmap
			// 2. Create a new pixmap and draw a coloured rectangle on it
			// 3. Blend these 2 pixmaps together by getting their pixbufs
			// 4. Draw the resulting pixbuf to become the new on-screen image
			GdkPixmap* opm = agtkw->get_ambulant_pixmap();
			gint width; gint height;
			gdk_drawable_get_size(GDK_DRAWABLE (opm), &width, &height);
			GdkPixmap* npm = gdk_pixmap_new(opm, width, height, -1);
			GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (opm));
			GdkGC *ngc = gdk_gc_new (GDK_DRAWABLE (npm));
			gdk_gc_set_rgb_fg_color (ngc, &bgc);
			gdk_draw_rectangle (GDK_DRAWABLE (npm), ngc, TRUE, L, T, W, H);
			GdkPixbuf* old_pixbuf = gdk_pixbuf_get_from_drawable(NULL, opm, NULL, L, T, 0, 0, W, H);
			GdkPixbuf* new_pixbuf = gdk_pixbuf_get_from_drawable(NULL, npm, NULL, L, T, 0, 0, W, H);
			int alpha = static_cast<int>(round(255*opacity));
			gdk_pixbuf_composite(new_pixbuf, old_pixbuf,0,0,W,H,0,0,1,1,GDK_INTERP_BILINEAR, alpha);
			gdk_draw_pixbuf(opm, gc, old_pixbuf, 0, 0, L, T, W, H, GDK_RGB_DITHER_NONE,0,0);
			g_object_unref (G_OBJECT (old_pixbuf));
			g_object_unref (G_OBJECT (new_pixbuf));
			g_object_unref (G_OBJECT (ngc));
			g_object_unref (G_OBJECT (gc));
		}
		//gtk_widget_modify_bg (GTK_WIDGET (agtkw->get_ambulant_widget()->get_gtk_widget()), GTK_STATE_NORMAL, &bgc );
		if (m_background_pixmap) {
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
	if (m_background_pixmap) {
		delete m_background_pixmap;
		m_background_pixmap = NULL;
	}
	m_background_pixmap = agtkw->get_pixmap_from_screen(dstrect_whole);
//XXXX	dumpPixmap(m_background_pixmap, "/tmp/keepbg");
}
