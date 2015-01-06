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

#include "ambulant/gui/gtk/gtk_image_renderer.h"
#include "ambulant/gui/gtk/gtk_transition.h"
#include "ambulant/gui/gtk/gtk_util.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/smil_alignment.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace common;
using namespace lib;
using namespace gui::gtk;

extern const char gtk_image_playable_tag[] = "img";
extern const char gtk_image_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererGtk");
extern const char gtk_image_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererImg");

common::playable_factory *
gui::gtk::create_gtk_image_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererGtk"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererImg"), true);
	return new common::single_playable_factory<
		gtk_image_renderer,
		gtk_image_playable_tag,
		gtk_image_playable_renderer_uri,
		gtk_image_playable_renderer_uri2,
		gtk_image_playable_renderer_uri2>(factory, mdp);
}

gtk_image_renderer::~gtk_image_renderer() {
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer::~gtk_image_renderer(0x%x), %s", this, this->get_sig().c_str());
	if (m_image)
		g_object_unref(G_OBJECT (m_image));
	m_lock.leave();
}
#if GTK_MAJOR_VERSION >= 3

void
gtk_image_renderer::redraw_body(const rect &dirty, gui_window* w) {

	m_lock.enter();
	const point p = m_dest->get_global_topleft();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x): m_image=0x%x, ltrb=(%d,%d,%d,%d), p=(%d,%d)", (void *)this, &m_image,r.left(), r.top(), r.right(), r.bottom(),p.x,p.y);

	const common::region_info *ri = m_dest->get_info();
	AM_DBG logger::get_logger()->debug("gtk_image_renderer.redraw_body: gui_window=0x%x info=0x%x",w,ri);
	ambulant_gtk_window* agtkw = (ambulant_gtk_window*) w;

	//
	// If the image is available but it hasn't been loaded yet: load it.
	//
	if (m_data && !m_image_loaded && m_data_size > 0) {
		GdkPixbufLoader *loader =  gdk_pixbuf_loader_new ();
		AM_DBG logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x): load data for %s", this, this->get_sig().c_str());
		if (gdk_pixbuf_loader_write(loader, (const guchar*) m_data, (gsize) m_data_size, 0))
		{
			// for small files (m_data_size < 128) gdk_pixbuf_loader_close() is needed
			// otherwise gdk_pixbuf_loader_get_pixbuf() doesn't get an image
			gdk_pixbuf_loader_close(loader, NULL);
			m_image = gdk_pixbuf_loader_get_pixbuf(loader);
			if (m_image)
				g_object_ref(G_OBJECT (m_image));
		}else
			g_message("Could not get Loader working\n");

		if (!m_image) {
			g_message ("Could not create the pixbuf\n");
		}else{
			m_image_loaded = TRUE;
		}
		if (loader) {
			g_object_unref(G_OBJECT (loader));
		}
	}
	//
	// If we don't have the image loaded: return. We'll be called again
	// when it has been loaded.
	//
	if ( ! m_image_loaded) {
		m_lock.leave();
		return;
	}

	int width = gdk_pixbuf_get_width(m_image);
	int height = gdk_pixbuf_get_height(m_image);
	size srcsize = size(width, height);
	rect srcrect;
	rect dstrect;

	// While rendering background images only, check for tiling. This code is
	// convoluted, it knows that the node and the region we're painting to are
	// really the same node.

	std::string id = m_dest->get_info()->get_name();
	AM_DBG logger::get_logger()->debug("%s: m_node=0x%x, m_dest=0x%x", __PRETTY_FUNCTION__, m_node, m_dest);
	cairo_t* cr = cairo_create(agtkw->get_target_surface());
	if (m_node != NULL && m_node->get_attribute("backgroundImage")) {
		AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw: drawing tiled image");
		// backgroundOpacity.
		double alpha = ri->get_bgopacity();
		dstrect = m_dest->get_rect();
		dstrect.translate(m_dest->get_global_topleft());
		// set surface to all tranparent pixels
		cairo_rectangle (cr, dstrect.left(), dstrect.top(), dstrect.width(), dstrect.height());
		cairo_clip(cr);
		cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
		cairo_paint (cr);
		// set surface to draw over existing pixels
		cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
		
		cairo_rectangle (cr, dstrect.left(), dstrect.top(), dstrect.width(), dstrect.height());
		cairo_clip(cr);
		if (m_dest->is_tiled()) {
			common::tile_positions tiles = m_dest->get_tiles(srcsize, dstrect);
			common::tile_positions::iterator it;

			for(it=tiles.begin(); it!=tiles.end(); it++) {

				srcrect = (*it).first;
				dstrect = (*it).second;
				int	S_L = srcrect.left(),
					S_T = srcrect.top(),
					S_W = srcrect.width(),
					S_H = srcrect.height();
				int	D_L = dstrect.left(),
					D_T = dstrect.top(),
					D_W = dstrect.width(),
					D_H = dstrect.height();
				AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x): drawImage at (L=%d,T=%d,W=%d,H=%d) from (L=%d,T=%d,W=%d,H=%d)",(void *)this,D_L,D_T,D_W,D_H,S_L,S_T,S_W,S_H);
//				cairo_rectangle (cr, D_L, D_T, D_W, D_H);
//				cairo_clip(cr);
				cairo_t* cr = cairo_create(agtkw->get_target_surface());
				gdk_cairo_set_source_pixbuf(cr, m_image, D_L, D_T);
				cairo_paint_with_alpha(cr, alpha);
			}
		} else {
			gdk_cairo_set_source_pixbuf(cr, m_image, dstrect.left(), dstrect.top());
			cairo_paint_with_alpha(cr, alpha);
		}
		cairo_destroy(cr);
		m_lock.leave();
		return;
	}
	srcrect = rect(size(0,0));
	lib::rect croprect = m_dest->get_crop_rect(srcsize);
	dstrect = m_dest->get_fit_rect(croprect, srcsize, &srcrect, m_alignment);
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
	// N_ for new (scaled) coordinates
	int	S_L = srcrect.left(),
		S_T = srcrect.top(),
		S_W = srcrect.width(),
		S_H = srcrect.height();
	int	D_L = dstrect.left(),
		D_T = dstrect.top(),
		D_W = dstrect.width(),
		D_H = dstrect.height();
	AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x): drawImage at (L=%d,T=%d,W=%d,H=%d) from (L=%d,T=%d,W=%d,H=%d), original(%d,%d)",(void *)this,D_L,D_T,D_W,D_H,S_L,S_T,S_W,S_H,width,height);
	float fact_W = (float)D_W/(float)S_W;
	float fact_H = (float)D_H/(float)S_H;
	// N_ for new (scaled) image coordinates
	int	N_L = (int)roundf(S_L*fact_W),
		N_T = (int)roundf(S_T*fact_H),
		N_W = (int)roundf(width*fact_W),
		N_H = (int)roundf(height*fact_H);
	AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x): orig=(%d, %d) scalex=%f, scaley=%f  intermediate (L=%d,T=%d,W=%d,H=%d) dest=(%d,%d,%d,%d)",(void *)this,width,height,fact_W,fact_H,N_L,N_T,N_W,N_H,D_L,D_T,D_W,D_H);
	
	
	GdkPixbuf* partial_pixbuf = gdk_pixbuf_new_subpixbuf(m_image, S_L, S_T, S_W, S_H);
	GdkPixbuf* new_image_pixbuf =  gdk_pixbuf_scale_simple(partial_pixbuf, D_W, D_H, GDK_INTERP_BILINEAR);
	g_object_unref(G_OBJECT(partial_pixbuf));
	N_L = N_T = 0;
	AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x): alpha_chroma=%f, alpha_media=%f, chrona_low=0x%x, chroma_high=0x%x", (void *)this, alpha_chroma, alpha_media, chroma_low, chroma_high);
	if (alpha_chroma != 1.0) {
		lib::rect rect0(lib::point(0,0),lib::size(D_W,D_H));
		//TBD
	} else {
		AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw_body(%p) target_surface=%p.",this,agtkw->get_target_surface());
		cairo_t* cr = cairo_create(agtkw->get_target_surface());
		gdk_cairo_set_source_pixbuf(cr, new_image_pixbuf, dstrect.left(), dstrect.top());
		cairo_paint_with_alpha(cr, alpha_media);
		cairo_destroy(cr);
	}
	g_object_unref(G_OBJECT(new_image_pixbuf));
	AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x done.", (void *)this);
	m_lock.leave();
}
#else //  GTK_MAJOR_VERSION < 3

void
gtk_image_renderer::redraw_body(const rect &dirty, gui_window* w) {

	m_lock.enter();
	const point p = m_dest->get_global_topleft();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x): m_image=0x%x, ltrb=(%d,%d,%d,%d), p=(%d,%d)", (void *)this, &m_image,r.left(), r.top(), r.right(), r.bottom(),p.x,p.y);

	const common::region_info *ri = m_dest->get_info();
	AM_DBG logger::get_logger()->debug("gtk_image_renderer.redraw_body: gui_window=0x%x info=0x%x",w,ri);
	ambulant_gtk_window* agtkw = (ambulant_gtk_window*) w;

	//
	// If the image is available but it hasn't been loaded yet: load it.
	//
	if (m_data && !m_image_loaded && m_data_size > 0) {
		GdkPixbufLoader *loader =  gdk_pixbuf_loader_new ();
		AM_DBG logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x): load data for %s", this, this->get_sig().c_str());
		if (gdk_pixbuf_loader_write(loader, (const guchar*) m_data, (gsize) m_data_size, 0))
		{
			// for small files (m_data_size < 128) gdk_pixbuf_loader_close() is needed
			// otherwise gdk_pixbuf_loader_get_pixbuf() doesn't get an image
			gdk_pixbuf_loader_close(loader, NULL);
			m_image = gdk_pixbuf_loader_get_pixbuf(loader);
			if (m_image)
				g_object_ref(G_OBJECT (m_image));
		}else
			g_message("Could not get Loader working\n");

		if (!m_image) {
			g_message ("Could not create the pixbuf\n");
		}else{
			m_image_loaded = TRUE;
		}
		if (loader) {
			g_object_unref(G_OBJECT (loader));
		}
	}
	//
	// If we don't have the image loaded: return. We'll be called again
	// when it has been loaded.
	//
	if ( ! m_image_loaded) {
		m_lock.leave();
		return;
	}

	int width = gdk_pixbuf_get_width(m_image);
	int height = gdk_pixbuf_get_height(m_image);
	size srcsize = size(width, height);
	rect srcrect;
	rect dstrect;

	// While rendering background images only, check for tiling. This code is
	// convoluted, it knows that the node and the region we're painting to are
	// really the same node.
	if (m_node->get_attribute("backgroundImage") && m_dest->is_tiled()) {
		AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw: drawing tiled image");
		dstrect = m_dest->get_rect();
		dstrect.translate(m_dest->get_global_topleft());
		common::tile_positions tiles = m_dest->get_tiles(srcsize, dstrect);
		common::tile_positions::iterator it;
		for(it=tiles.begin(); it!=tiles.end(); it++) {

			srcrect = (*it).first;
			dstrect = (*it).second;
			int S_L = srcrect.left(),
				S_T = srcrect.top(),
				S_W = srcrect.width(),
				S_H = srcrect.height();
			int D_L = dstrect.left(),
				D_T = dstrect.top(),
				D_W = dstrect.width(),
				D_H = dstrect.height();
			AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x): drawImage at (L=%d,T=%d,W=%d,H=%d) from (L=%d,T=%d,W=%d,H=%d)",(void *)this,D_L,D_T,D_W,D_H,S_L,S_T,S_W,S_H);
			GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()));
			gdk_pixbuf_render_to_drawable(
				m_image,
				GDK_DRAWABLE (agtkw->get_ambulant_pixmap()),
				gc,
				S_L, S_T,
				D_L, D_T,
				D_W, D_H,
				GDK_RGB_DITHER_NONE,
				0,
				0);
			g_object_unref (G_OBJECT(m_image));
			g_object_unref (G_OBJECT (gc));
		}
		m_lock.leave();
		return;
	}
	
	srcrect = rect(size(0,0));
	lib::rect croprect = m_dest->get_crop_rect(srcsize);
	dstrect = m_dest->get_fit_rect(croprect, srcsize, &srcrect, m_alignment);
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
	// N_ for new (scaled) coordinates
	int	S_L = srcrect.left(),
		S_T = srcrect.top(),
		S_W = srcrect.width(),
		S_H = srcrect.height();
	int	D_L = dstrect.left(),
		D_T = dstrect.top(),
		D_W = dstrect.width(),
		D_H = dstrect.height();
	AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x): drawImage at (L=%d,T=%d,W=%d,H=%d) from (L=%d,T=%d,W=%d,H=%d), original(%d,%d)",(void *)this,D_L,D_T,D_W,D_H,S_L,S_T,S_W,S_H,width,height);
	float fact_W = (float)D_W/(float)S_W;
	float fact_H = (float)D_H/(float)S_H;
	// N_ for new (scaled) image coordinates
	int	N_L = (int)roundf(S_L*fact_W),
		N_T = (int)roundf(S_T*fact_H),
		N_W = (int)roundf(width*fact_W),
		N_H = (int)roundf(height*fact_H);
	AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x): orig=(%d, %d) scalex=%f, scaley=%f  intermediate (L=%d,T=%d,W=%d,H=%d) dest=(%d,%d,%d,%d)",(void *)this,width,height,fact_W,fact_H,N_L,N_T,N_W,N_H,D_L,D_T,D_W,D_H);
	GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()));
	
	
	GdkPixbuf* partial_pixbuf = gdk_pixbuf_new_subpixbuf(m_image, S_L, S_T, S_W, S_H);
	GdkPixbuf* new_image_pixbuf =  gdk_pixbuf_scale_simple(partial_pixbuf, D_W, D_H, GDK_INTERP_BILINEAR);
	g_object_unref(G_OBJECT(partial_pixbuf));
	N_L = N_T = 0;
	AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x): alpha_chroma=%f, alpha_media=%f, chrona_low=0x%x, chroma_high=0x%x", (void *)this, alpha_chroma, alpha_media, chroma_low, chroma_high);
	if (alpha_chroma != 1.0) {
		lib::rect rect0(lib::point(0,0),lib::size(D_W,D_H));

		GdkPixbuf* screen_pixbuf = gdk_pixbuf_get_from_drawable (
			NULL,
			agtkw->get_ambulant_pixmap(),
			NULL,
			D_L, D_T,
			0, 0,
			D_W, D_H);
		gdk_pixbuf_blend (
			screen_pixbuf,
			rect0,
			new_image_pixbuf,
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
			new_image_pixbuf,
			N_L, N_T,
			D_L, D_T,
			D_W, D_H,
			GDK_RGB_DITHER_NONE, 0, 0);
	}
	g_object_unref(G_OBJECT(gc));
	g_object_unref(G_OBJECT(new_image_pixbuf));
	AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x done.", (void *)this);
	m_lock.leave();
}
#endif //  GTK_MAJOR_VERSION < 3
