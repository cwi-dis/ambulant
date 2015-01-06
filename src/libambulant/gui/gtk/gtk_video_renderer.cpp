/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

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

#include "ambulant/gui/gtk/gtk_factory.h"
#include "ambulant/gui/gtk/gtk_renderer.h"
#include "ambulant/gui/gtk/gtk_video_renderer.h"
#include "ambulant/gui/gtk/gtk_util.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/factory.h"
#include <stdlib.h>
#include "ambulant/common/playable.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

// XXXJACK: we should get rid of these, analoguous to what cocoa_dsvideo does:
// Get the information dynamically.
#if 1
#define MY_PIXEL_LAYOUT net::pixel_abgr
#define MY_HASALPHA TRUE
#define MY_BPP 4
#endif
#if 0
#define MY_PIXEL_LAYOUT net::pixel_rgb
#define MY_HASALPHA FALSE
#define MY_BPP 3
#endif

using namespace ambulant;
using namespace gui::gtk;

extern const char gtk_video_playable_tag[] = "video";
extern const char gtk_video_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererGtk");
extern const char gtk_video_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererVideo");
extern const char gtk_video_playable_renderer_uri3[] = AM_SYSTEM_COMPONENT("RendererOpen");

common::playable_factory *
gui::gtk::create_gtk_video_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererGtk"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererVideo"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererOpen"), true);
	return new common::single_playable_factory<
		gtk_video_renderer,
		gtk_video_playable_tag,
		gtk_video_playable_renderer_uri,
		gtk_video_playable_renderer_uri2,
		gtk_video_playable_renderer_uri3>(factory, mdp);
}

gtk_video_renderer::gtk_video_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp)
:	gtk_renderer<common::video_renderer>(context, cookie, node, evp, factory, mdp),
	m_image(NULL),
	m_data(NULL),
	m_img_displayed(0)
{
	assert(m_frames.size() == 0);

}

gtk_video_renderer::~gtk_video_renderer()
{
	m_lock.enter();
	if (m_data)
		free(m_data);
	if(m_image)
		g_object_unref (G_OBJECT (m_image));
	m_lock.leave();
}

net::pixel_order
gtk_video_renderer::pixel_layout()
{
	return MY_PIXEL_LAYOUT;
}

void
gtk_video_renderer::_push_frame(char* frame, size_t size)
{

	AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer::_push_frame: frame=0x%x, size=%d, this=0x%x", (void*) frame, size, (void*) this);
	if (m_data)
		free(m_data);
	m_data = frame;
}


#if GTK_MAJOR_VERSION >= 3

void
gtk_video_renderer::redraw_body(const lib::rect &dirty, common::gui_window* w)
{
	//XXXX locking at this point may result in deadly embrace with internal lock,
	//XXXX but as far as we know this has never happened
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw(0x%x)",(void*) this);
	if (m_data == NULL) {
		AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw(0x%x): no video image data",(void*) this);
		m_lock.leave();
		return;
	}
	_frame_was_displayed();
	const lib::point p = m_dest->get_global_topleft();
	const lib::rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("gtk_video_renderer.redraw_body(0x%x): m_data=0x%x, ltrb=(%d,%d,%d,%d), p=(%d,%d)", (void *)this, &m_image,r.left(), r.top(), r.right(), r.bottom(),p.x,p.y);

	const common::region_info *ri = m_dest->get_info();
	AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw: info=0x%x", ri);
	ambulant_gtk_window* agtkw = (ambulant_gtk_window*) w;
	//
	// Convert to a gdk_pixbuf
	//
	AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw_body(0x%x): width = %d, height = %d",(void *)this, m_size.w, m_size.h);
	m_image =  gdk_pixbuf_new_from_data ((const guchar*) m_data, GDK_COLORSPACE_RGB, MY_HASALPHA, 8, m_size.w, m_size.h, (m_size.w*MY_BPP), NULL, NULL);

	if (m_image == NULL) {
		lib::logger::get_logger()->debug("gtk_video_renderer.redraw_body(0x%x): gdk_pixbuf_new_from_data returned NULL",(void *)this);
		m_lock.leave();
		return;
	}
	
	int width = gdk_pixbuf_get_width(m_image);
	int height = gdk_pixbuf_get_height(m_image);
	size srcsize = size(width, height);
	lib::rect croprect = m_dest->get_crop_rect(srcsize);
	lib::rect srcrect = lib::rect(lib::size(0,0));
	lib::rect dstrect = m_dest->get_fit_rect(srcsize, &srcrect, m_alignment);
	dstrect.translate(m_dest->get_global_topleft());
	
	double alpha_media = 1.0, alpha_media_bg = 1.0, alpha_chroma = 1.0;
	lib::color_t chroma_low = lib::color_t(0x000000), chroma_high = lib::color_t(0xFFFFFF);
	if (ri) {
		alpha_media = ri->get_mediaopacity();
		if (ri->is_chromakey_specified()) {
			alpha_chroma = ri->get_chromakeyopacity();
			lib::color_t chromakey = ri->get_chromakey();
			lib::color_t chromakeytolerance = ri->get_chromakeytolerance();
			compute_chroma_range(chromakey, chromakeytolerance, &chroma_low, &chroma_high);
		}
	}	
	// S_ for source image coordinates
	// D_ for destination coordinates
	int S_L = srcrect.left(),
		S_T = srcrect.top(),
		S_W = srcrect.width(),
		S_H = srcrect.height();
	int D_L = dstrect.left(),
		D_T = dstrect.top(),
		D_W = dstrect.width(),
		D_H = dstrect.height();
	AM_DBG lib::logger::get_logger()->debug("gtk_videorenderer.redraw_body(0x%x): gtk_draw_pixbuf at (L=%d,T=%d,W=%d,H=%d) from (L=%d,T=%d,W=%d,H=%d), original(%d,%d)",(void *)this,D_L,D_T,D_W,D_H,S_L,S_T,S_W,S_H,width,height);
	float fact_W = (float)D_W/(float)S_W;
	float fact_H = (float)D_H/(float)S_H;

	// N_ for new (scaled) image coordinates
	int N_L = (int)roundf(S_L*fact_W),
		N_T = (int)roundf(S_T*fact_H),
		N_W = (int)roundf(width*fact_W),
		N_H = (int)roundf(height*fact_H);
	AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw_body(0x%x): orig=(%d, %d) scalex=%f, scaley=%f  intermediate (L=%d,T=%d,W=%d,H=%d) dest=(%d,%d,%d,%d)",(void *)this,width,height,fact_W,fact_H,N_L,N_T,N_W,N_H,D_L,D_T,D_W,D_H);
	GdkPixbuf* new_image_pixbuf = NULL;
	if (S_L != 0 || S_T != 0 || S_W != D_W || S_H != D_H) {
		GdkPixbuf* partial_pixbuf = gdk_pixbuf_new_subpixbuf(m_image, S_L, S_T, S_W, S_H);
		new_image_pixbuf = gdk_pixbuf_scale_simple(partial_pixbuf, D_W, D_H, GDK_INTERP_BILINEAR);
		g_object_unref(G_OBJECT(partial_pixbuf));
	}
	N_L = N_T = 0;
	AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw_body(0x%x): alpha_chroma=%f, alpha_media=%f, chrona_low=0x%x, chroma_high=0x%x", (void *)this, alpha_chroma, alpha_media, chroma_low, chroma_high);
// ignore chroma, for now
//	if (alpha_chroma != 1.0) {
//	} else {
		cairo_t* cr = cairo_create(agtkw->get_target_surface());
		gdk_cairo_set_source_pixbuf(cr, new_image_pixbuf == NULL ? m_image : new_image_pixbuf, dstrect.left(), dstrect.top());
		cairo_paint_with_alpha(cr, alpha_media);
		cairo_destroy(cr);
//	}
	if (new_image_pixbuf != NULL) {
		g_object_unref(G_OBJECT (new_image_pixbuf));
	}
	AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw_body(0x%x done.", (void *)this);
	m_lock.leave();
}
#else // GTK_MAJOR_VERSION < 3

void
gtk_video_renderer::redraw_body(const lib::rect &dirty, common::gui_window* w)
{
	//XXXX locking at this point may result in deadly embrace with internal lock,
	//XXXX but as far as we know this has never happened
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw(0x%x)",(void*) this);
	if (m_data == NULL) {
		AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw(0x%x): no video image data",(void*) this);
		m_lock.leave();
		return;
	}
	_frame_was_displayed();
	const lib::point p = m_dest->get_global_topleft();
	const lib::rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("gtk_video_renderer.redraw_body(0x%x): m_data=0x%x, ltrb=(%d,%d,%d,%d), p=(%d,%d)", (void *)this, &m_image,r.left(), r.top(), r.right(), r.bottom(),p.x,p.y);

	const common::region_info *ri = m_dest->get_info();
	AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw: info=0x%x", ri);
	ambulant_gtk_window* agtkw = (ambulant_gtk_window*) w;
	// Jack thinks this isn't needed... Done by the background draw method...
	// background drawing
	if (ri && (ri->get_bgopacity() > 0.5)) {
		// First find our whole area (which we have to clear to
		// background color)
		lib::rect dstrect_whole = r;
		dstrect_whole.translate(m_dest->get_global_topleft());
		int L = dstrect_whole.left(),
			T = dstrect_whole.top(),
			W = dstrect_whole.width(),
			H = dstrect_whole.height();
		// XXXX Fill with background color
		lib::color_t bgcolor = ri->get_bgcolor();
		AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw: clearing to 0x%x", (long)bgcolor);
		GdkColor bgc;
		bgc.red = redc(bgcolor)*0x101;
		bgc.blue = bluec(bgcolor)*0x101;
		bgc.green = greenc(bgcolor)*0x101;
		GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()));
		gdk_gc_set_rgb_fg_color (gc, &bgc);
		gdk_draw_rectangle (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()), gc, TRUE, L, T, W, H);
		g_object_unref (G_OBJECT (gc));
	}

	//
	// Convert to a gdk_pixbuf
	//
	AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw_body(0x%x): width = %d, height = %d",(void *)this, m_size.w, m_size.h);
	m_image =  gdk_pixbuf_new_from_data ((const guchar*) m_data, GDK_COLORSPACE_RGB, MY_HASALPHA, 8, m_size.w, m_size.h, (m_size.w*MY_BPP), NULL, NULL);

	if (m_image == NULL) {
		lib::logger::get_logger()->debug("gtk_video_renderer.redraw_body(0x%x): gdk_pixbuf_new_from_data returned NULL",(void *)this);
		m_lock.leave();
		return;
	}
	
	int width = gdk_pixbuf_get_width(m_image);
	int height = gdk_pixbuf_get_height(m_image);
	size srcsize = size(width, height);
	lib::rect croprect = m_dest->get_crop_rect(srcsize);
	lib::rect srcrect = lib::rect(lib::size(0,0));
	lib::rect dstrect = m_dest->get_fit_rect(srcsize, &srcrect, m_alignment);
	dstrect.translate(m_dest->get_global_topleft());
	
	double alpha_media = 1.0, alpha_media_bg = 1.0, alpha_chroma = 1.0;
	lib::color_t chroma_low = lib::color_t(0x000000), chroma_high = lib::color_t(0xFFFFFF);
	if (ri) {
		alpha_media = ri->get_mediaopacity();
		if (ri->is_chromakey_specified()) {
			alpha_chroma = ri->get_chromakeyopacity();
			lib::color_t chromakey = ri->get_chromakey();
			lib::color_t chromakeytolerance = ri->get_chromakeytolerance();
			compute_chroma_range(chromakey, chromakeytolerance, &chroma_low, &chroma_high);
		} else alpha_chroma = alpha_media;
	}	
	// S_ for source image coordinates
	// D_ for destination coordinates
	int S_L = srcrect.left(),
		S_T = srcrect.top(),
		S_W = srcrect.width(),
		S_H = srcrect.height();
	int D_L = dstrect.left(),
		D_T = dstrect.top(),
		D_W = dstrect.width(),
		D_H = dstrect.height();
	AM_DBG lib::logger::get_logger()->debug("gtk_videorenderer.redraw_body(0x%x): gtk_draw_pixbuf at (L=%d,T=%d,W=%d,H=%d) from (L=%d,T=%d,W=%d,H=%d), original(%d,%d)",(void *)this,D_L,D_T,D_W,D_H,S_L,S_T,S_W,S_H,width,height);
	float fact_W = (float)D_W/(float)S_W;
	float fact_H = (float)D_H/(float)S_H;

	// N_ for new (scaled) image coordinates
	int N_L = (int)roundf(S_L*fact_W),
		N_T = (int)roundf(S_T*fact_H),
		N_W = (int)roundf(width*fact_W),
		N_H = (int)roundf(height*fact_H);
	AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw_body(0x%x): orig=(%d, %d) scalex=%f, scaley=%f  intermediate (L=%d,T=%d,W=%d,H=%d) dest=(%d,%d,%d,%d)",(void *)this,width,height,fact_W,fact_H,N_L,N_T,N_W,N_H,D_L,D_T,D_W,D_H);
	GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()));
	GdkPixbuf* new_image_pixbuf = NULL;
	if (S_L != 0 || S_T != 0 || S_W != D_W || S_H != D_H) {
		GdkPixbuf* partial_pixbuf = gdk_pixbuf_new_subpixbuf(m_image, S_L, S_T, S_W, S_H);
		new_image_pixbuf = gdk_pixbuf_scale_simple(partial_pixbuf, D_W, D_H, GDK_INTERP_BILINEAR);
		g_object_unref(G_OBJECT(partial_pixbuf));
	}
	N_L = N_T = 0;
	AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw_body(0x%x): alpha_chroma=%f, alpha_media=%f, chrona_low=0x%x, chroma_high=0x%x", (void *)this, alpha_chroma, alpha_media, chroma_low, chroma_high);
	if (alpha_chroma != 1.0) {
		GdkPixbuf* screen_pixbuf = gdk_pixbuf_get_from_drawable (
			NULL,
			agtkw->get_ambulant_pixmap(),
			NULL,
			D_L, D_T,
			0, 0,
			D_W, D_H);
		lib::rect rect0(lib::point(0,0),lib::size(D_W,D_H));
		gdk_pixbuf_blend (
			screen_pixbuf,
			rect0,
			new_image_pixbuf == NULL ? m_image : new_image_pixbuf,
			rect0,
			alpha_chroma,
			alpha_media,
			chroma_low,
			chroma_high);
		gdk_draw_pixbuf(GDK_DRAWABLE (
			agtkw->get_ambulant_pixmap()),
			gc,
			screen_pixbuf,
			N_L, N_T,
			D_L, D_T,
			D_W, D_H,
			GDK_RGB_DITHER_NONE, 0, 0);
	} else {
		gdk_draw_pixbuf(
			GDK_DRAWABLE(agtkw->get_ambulant_pixmap()),
			gc,
			new_image_pixbuf == NULL ? m_image : new_image_pixbuf,
			N_L, N_T,
			D_L, D_T,
			D_W, D_H,
			GDK_RGB_DITHER_NONE, 0, 0);
	}
	if (new_image_pixbuf != NULL) {
		g_object_unref(G_OBJECT (new_image_pixbuf));
	}
	g_object_unref (G_OBJECT (gc));
	AM_DBG lib::logger::get_logger()->debug("gtk_video_renderer.redraw_body(0x%x done.", (void *)this);
	m_lock.leave();
}
#endif // GTK_MAJOR_VERSION < 3
