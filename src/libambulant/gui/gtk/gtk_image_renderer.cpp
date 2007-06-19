// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2007 Stichting CWI, 
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

/* 
 * @$Id$
 */

#include "ambulant/gui/gtk/gtk_includes.h"
#include "ambulant/gui/gtk/gtk_image_renderer.h"
#include "ambulant/gui/gtk/gtk_transition.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/smil_alignment.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace common;
using namespace lib;
using namespace gui::gtk;


gtk_image_renderer::~gtk_image_renderer() {
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer::~gtk_image_renderer(0x%x)", this);
	if (m_image)
		g_object_unref(G_OBJECT (m_image));
	m_lock.leave();
}
	
void
gtk_image_renderer::redraw_body(const rect &dirty,
				      gui_window* w) {

	m_lock.enter();
	const point             p = m_dest->get_global_topleft();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x): m_image=0x%x, ltrb=(%d,%d,%d,%d), p=(%d,%d)", (void *)this, &m_image,r.left(), r.top(), r.right(), r.bottom(),p.x,p.y);

// XXXX WRONG! This is the info for the region, not for the node!
	const common::region_info *info = m_dest->get_info();
	AM_DBG logger::get_logger()->debug("gtk_image_renderer.redraw_body: gui_window=0x%x info=0x%x",w,info);
	ambulant_gtk_window* agtkw = (ambulant_gtk_window*) w;

	if (m_data && !m_image_loaded && m_data_size > 0) {
		GdkPixbufLoader *loader =  gdk_pixbuf_loader_new ();
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
	if ( ! m_image_loaded) {
		// Initially the image may not yet be loaded
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
			int	S_L = srcrect.left(), 
				S_T = srcrect.top(),
				S_W = srcrect.width(),
		        	S_H = srcrect.height();
			int	D_L = dstrect.left(), 
				D_T = dstrect.top(),
				D_W = dstrect.width(),
				D_H = dstrect.height();
			AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x): drawImage at (L=%d,T=%d,W=%d,H=%d) from (L=%d,T=%d,W=%d,H=%d)",(void *)this,D_L,D_T,D_W,D_H,S_L,S_T,S_W,S_H);
			GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()));
			gdk_pixbuf_render_to_drawable(m_image,
                                             GDK_DRAWABLE (agtkw->get_ambulant_pixmap()), gc, S_L, S_T, D_L, D_T, D_W, D_H, GDK_RGB_DITHER_NONE, 0, 0);
			gdk_pixbuf_unref (m_image);
			g_object_unref (G_OBJECT (gc));			
		}
		m_lock.leave();
		return;
	}
	srcrect = rect(size(0,0));

#ifdef	WITH_SMIL30
	lib::rect croprect = m_dest->get_crop_rect(srcsize);
	dstrect = m_dest->get_fit_rect(croprect, srcsize, &srcrect, m_alignment);
#else //WITH_SMIL30
	dstrect = m_dest->get_fit_rect(srcsize, &srcrect, m_alignment);
#endif//WITH_SMIL30
	dstrect.translate(m_dest->get_global_topleft());
	// S_ for source image coordinates
	// D_ for destination coordinates
	int	S_L = srcrect.left(), 
		S_T = srcrect.top(),
		S_W = srcrect.width(),
		S_H = srcrect.height();
	int	D_L = dstrect.left(), 
		D_T = dstrect.top(),
		D_W = dstrect.width(),
		D_H = dstrect.height();
	AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x): drawImage at (L=%d,T=%d,W=%d,H=%d) from (L=%d,T=%d,W=%d,H=%d), original(%d,%d)",(void *)this,D_L,D_T,D_W,D_H,S_L,S_T,S_W,S_H,width,height);

	GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()));
#ifdef	WITH_SMIL30
	/* scale image s.t. the viewbox specified fits in destination area:
	 * zoom_X=(width/S_W), fit_X=(D_W/width); scale_X=zoom_X*fit_X  */
	double scale_X = (double)D_W/(double)S_W;
	double scale_Y = (double)D_H/(double)S_H;
	double off_X = S_L*scale_X;
	double off_Y = S_T*scale_Y;
	GdkPixbuf* new_image = gdk_pixbuf_copy(m_image);
	AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x):gdk_pixbuf_scale(dest=%d,%d,%d,%d offs=%lf,%lf scal=%lf, %lf)",(void *)this,0,0,D_W,D_H,-off_X,-off_Y,scale_X,scale_Y);
	gdk_pixbuf_scale(m_image, new_image, 0, 0, D_W, D_H, -off_X, -off_Y, scale_X, scale_Y, GDK_INTERP_BILINEAR);
	AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x):gdk_draw_pixbuf(src=%d,%d dest=%d,%d,%d,%d)",(void *)this,0,0,D_L,D_T,D_W,D_H);
	gdk_draw_pixbuf(GDK_DRAWABLE (agtkw->get_ambulant_pixmap()), gc, new_image,0, 0, D_L, D_T, D_W, D_H, GDK_RGB_DITHER_NONE, 0, 0);
#else //WITH_SMIL30
	GdkPixbuf* new_image = gdk_pixbuf_scale_simple(m_image, D_W, D_H, GDK_INTERP_BILINEAR);
	gdk_draw_pixbuf(GDK_DRAWABLE (agtkw->get_ambulant_pixmap()), gc, new_image, 0, 0, D_L, D_T, D_W, D_H, GDK_RGB_DITHER_NONE, 0, 0);
#endif//WITH_SMIL30
	g_object_unref(G_OBJECT (new_image));
	g_object_unref(G_OBJECT (gc));
	m_lock.leave();
}
