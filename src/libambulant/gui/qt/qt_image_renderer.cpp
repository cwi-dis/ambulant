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

#include "ambulant/gui/qt/qt_includes.h"
#include "ambulant/gui/qt/qt_image_renderer.h"
#include "ambulant/gui/qt/qt_transition.h"
#include "ambulant/gui/qt/qt_util.h"
#include "ambulant/common/region_info.h"
#include "ambulant/common/smil_alignment.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace common;
using namespace lib;
using namespace gui::qt;

qt_image_renderer::~qt_image_renderer() {
	m_lock.enter();
	AM_DBG lib::logger::get_logger()->debug("qt_image_renderer::~qt_image_renderer(0x%x)", this);
	m_lock.leave();
}

void
qt_image_renderer::redraw_body(const rect &dirty,
				      gui_window* w) {
	m_lock.enter();
	const point             p = m_dest->get_global_topleft();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("qt_image_renderer.redraw_body(0x%x): m_image=0x%x, ltrb=(%d,%d,%d,%d), p=(%d,%d)", (void *)this, &m_image,r.left(), r.top(), r.right(), r.bottom(),p.x,p.y);
	if (m_data && !m_image_loaded) {
		m_image_loaded = m_image.loadFromData((const uchar*)m_data, m_data_size);
	}
	if ( ! m_image_loaded) {
		// Initially the image may not yet be loaded
	 	m_lock.leave();
		return;
	}
// XXXX WRONG! This is the info for the region, not for the node!
	const common::region_info *info = m_dest->get_info();
	AM_DBG logger::get_logger()->debug("qt_image_renderer.redraw_body: info=0x%x",info);
	ambulant_qt_window* aqw = (ambulant_qt_window*) w;

	QPainter paint;
	paint.begin(aqw->get_ambulant_pixmap());
	QSize qsize = m_image.size();
	size srcsize = size(qsize.width(), qsize.height());
	rect srcrect;
	rect dstrect;

	// While rendering background images only, check for tiling. This code is
	// convoluted, it knows that the node and the region we're painting to are
	// really the same node.
	if (m_node->get_attribute("backgroundImage") && m_dest->is_tiled()) {
		AM_DBG lib::logger::get_logger()->debug("qt_image_renderer.redraw: drawing tiled image");
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
			AM_DBG lib::logger::get_logger()->debug("qt_image_renderer.redraw_body(0x%x): drawImage at (L=%d,T=%d,W=%d,H=%d) from (L=%d,T=%d,W=%d,H=%d)",(void *)this,D_L,D_T,D_W,D_H,S_L,S_T,S_W,S_H);
			paint.drawImage(D_L,D_T, m_image, S_L,S_T, S_W,S_H);
	
		}
		paint.flush();
		paint.end();
		m_lock.leave();
		return;
	}

	srcrect = rect(size(0,0));
#ifdef	WITH_SMIL30
	lib::rect croprect = m_dest->get_crop_rect(srcsize);
	dstrect = m_dest->get_fit_rect(croprect, srcsize, &srcrect, m_alignment);
	double alpha_media = 1.0, alpha_media_bg = 1.0, alpha_chroma = 1.0;
	lib::color_t chroma_low = lib::color_t(0x000000), chroma_high = lib::color_t(0xFFFFFF);
	const common::region_info *ri = m_dest->get_info();
	if (ri) {
		alpha_media = ri->get_mediaopacity();
//???		alpha_media_bg = ri->get_mediabgopacity();
//???		m_bgopacity = ri->get_bgopacity();
		if (ri->is_chromakey_specified()) {
			alpha_chroma = ri->get_chromakeyopacity();
			lib::color_t chromakey = ri->get_chromakey();
			lib::color_t chromakeytolerance = ri->get_chromakeytolerance();
			compute_chroma_range(chromakey, chromakeytolerance,
					     &chroma_low, &chroma_high);
		} else alpha_chroma = alpha_media;
	}
#else //WITH_SMIL30
	dstrect = m_dest->get_fit_rect(srcsize, &srcrect, m_alignment);
#endif//WITH_SMIL30
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
	AM_DBG lib::logger::get_logger()->debug("qt_image_renderer.redraw_body(0x%x): drawImage at (L=%d,T=%d,W=%d,H=%d) from (L=%d,T=%d,W=%d,H=%d)",(void *)this,D_L,D_T,D_W,D_H,S_L,S_T,S_W,S_H);
	/* scale image s.t. the viewbox specified fits in destination area:
	 * zoom_X=(O_W/S_W), fit_X=(D_W/O_W); fact_W=zoom_X*fit_X  */
	float	fact_W = (float)D_W/(float)S_W,
		fact_H = (float)D_H/(float)S_H;
	int	N_L = (int)roundf(S_L*fact_W),
		N_T = (int)roundf(S_T*fact_H),
		N_W = (int)roundf(O_W*fact_W),
		N_H = (int)roundf(O_H*fact_H);
	AM_DBG lib::logger::get_logger()->debug("qt_image_renderer.redraw_body(0x%x): orig=(%d, %d) scalex=%f, scaley=%f  intermediate (L=%d,T=%d,W=%d,H=%d)",(void *)this,O_W,O_H,fact_W,fact_H,N_L,N_T,N_W,N_H);
#ifndef QT_NO_FILEDIALOG	/* Assume plain Qt */
	QImage scaledimage = m_image.smoothScale(N_W, N_H, QImage::ScaleFree);
#else /*QT_NO_FILEDIALOG*/	/* Assume embedded Qt */
	QImage scaledimage = m_image.smoothScale(N_W, N_H);
#endif/*QT_NO_FILEDIALOG*/
#ifdef	WITH_SMIL30
	if (alpha_chroma != 1.0) {
		QImage screen_img = aqw->get_ambulant_pixmap()->convertToImage();
		lib::rect rct0 (lib::point(0, 0), lib::size(N_W, N_H));
		qt_image_blend (screen_img, dstrect, scaledimage, rct0, 
				alpha_chroma, alpha_media,
				chroma_low, chroma_high);
		paint.drawImage(D_L, D_T, screen_img, D_L, D_T, D_W, D_H);
	} else {
		paint.drawImage(D_L, D_T, scaledimage, N_L, N_T, D_W, D_H);
	}
#else //WITH_SMIL30		
	paint.drawImage(D_L, D_T, scaledimage, N_L, N_T, D_W, D_H);
#endif//WITH_SMIL30
	paint.flush();
	paint.end();

	m_lock.leave();
}
