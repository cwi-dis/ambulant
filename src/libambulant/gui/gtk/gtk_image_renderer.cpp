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

/* 
 * @$Id$
 */

#include "ambulant/gui/gtk/gtk_includes.h"
#include "ambulant/gui/gtk/gtk_image_renderer.h"
#include "ambulant/gui/gtk/gtk_transition.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/smil_alignment.h"

#define AM_DBG
//#ifndef AM_DBG
//#define AM_DBG if(0)
//#endif

using namespace ambulant;
using namespace common;
using namespace lib;
using namespace gui::gtk;

gtk_image_renderer::~gtk_image_renderer() {
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer::~gtk_image_renderer(0x%x)", this);
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
	AM_DBG logger::get_logger()->debug("gtk_image_renderer.redraw_body: info=0x%x",info);
	ambulant_gtk_window* agtkw = (ambulant_gtk_window*) w;

	if (m_data && !m_image_loaded) {
		m_image = gdk_pixmap_create_from_data(
					GDK_DRAWABLE(agtkw->get_ambulant_pixmap()),
					(const gchar*) m_data,
					r.width(),
					r.height(),
					-1,
					&agtkw->get_ambulant_widget()->get_gtk_widget()->style->black,
					&agtkw->get_ambulant_widget()->get_gtk_widget()->style->white);
		if (m_image)
			m_image_loaded = TRUE;
//m_image_loaded = m_image.loadFromData((const uchar*)m_data, m_data_size);
	}
	if ( ! m_image_loaded) {
		// Initially the image may not yet be loaded
	 	m_lock.leave();
		return;
	}



	int width; int height;
	gdk_drawable_get_size(m_image, &width, &height);
	printf("The sizes are: %i %i\n",width,height);
	size srcsize = size(width, height);
	rect srcrect;
	rect dstrect;
#ifdef USE_SMIL21
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
			gdk_draw_drawable (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()), gc, m_image, D_L,D_T, S_L,S_T, S_W,S_H);
			g_object_unref (G_OBJECT (gc));			
		}
		m_lock.leave();
		return;
	}
#endif
	srcrect = rect(size(0,0));
	dstrect = m_dest->get_fit_rect(srcsize, &srcrect, m_alignment);
	dstrect.translate(m_dest->get_global_topleft());
	// O_ for original image coordinates
	// S_ for source image coordinates
	// N_ for new (scaled) image coordinates
	// D_ for destination coordinates
	int	O_W = srcsize.w,
		O_H = srcsize.h;
	int	S_L = srcrect.left(), 
		S_T = srcrect.top(),
		S_W = srcrect.width(),
		S_H = srcrect.height();
	int	D_L = dstrect.left(), 
		D_T = dstrect.top(),
		D_W = dstrect.width(),
		D_H = dstrect.height();
	AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x): drawImage at (L=%d,T=%d,W=%d,H=%d) from (L=%d,T=%d,W=%d,H=%d)",(void *)this,D_L,D_T,D_W,D_H,S_L,S_T,S_W,S_H);
	float	fact_W = (float)D_W/(float)S_W,
		fact_H = (float)D_H/(float)S_H;
	int	N_L = (int)(S_L*fact_W),
		N_T = (int)(S_T*fact_H),
		N_W = (int)(O_W*fact_W),
		N_H = (int)(O_H*fact_H);
	AM_DBG lib::logger::get_logger()->debug("gtk_image_renderer.redraw_body(0x%x): orig=(%d, %d) scalex=%f, scaley=%f  intermediate (L=%d,T=%d,W=%d,H=%d)",(void *)this,O_W,O_H,fact_W,fact_H,N_L,N_T,N_W,N_H);
	GdkGC *gc = gdk_gc_new (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()));
	gdk_draw_drawable (GDK_DRAWABLE (agtkw->get_ambulant_pixmap()), gc, m_image, D_L,D_T, S_L, S_T, S_W, S_H);
	g_object_unref (G_OBJECT (gc));

//#ifndef QT_NO_FILEDIALOG*/	/* Assume plain Qt */
//	QImage scaledimage = m_image.smoothScale(N_W, N_H, QImage::ScaleFree);
//#else /*QT_NO_FILEDIALOG*/	/* Assume embedded Qt */
//	QImage scaledimage = m_image.smoothScale(N_W, N_H);
//#endif/*QT_NO_FILEDIALOG*/
//	paint.drawImage(D_L, D_T, scaledimage, N_L, N_T, D_W,D_H);
	m_lock.leave();
}
