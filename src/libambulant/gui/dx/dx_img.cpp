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

#include "ambulant/gui/dx/dx_img.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_image_renderer.h"
#include "ambulant/gui/dx/dx_transition.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/memfile.h"
#include "ambulant/lib/logger.h"

#include "ambulant/common/region_info.h"

#include <math.h>
#include <ddraw.h>

//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dx::dx_img_renderer::dx_img_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::factories *factory,
	dx_playables_context *dxplayer)
:   dx_renderer_playable(context, cookie, node, evp, dxplayer),
	m_image(0),
	m_factory(factory) {
	
	AM_DBG lib::logger::get_logger()->debug("dx_img_renderer::ctr(0x%x)", this);
}

gui::dx::dx_img_renderer::~dx_img_renderer() {
	AM_DBG lib::logger::get_logger()->debug("dx_img_renderer::dtr(0x%x)", this);
	delete m_image;
}


void gui::dx::dx_img_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("dx_img_renderer::start(0x%x)", this);
	net::url url = m_node->get_url("src");
	net::datasource *src = m_factory->get_datasource_factory()->new_raw_datasource(url);
	if (src == NULL) {
		m_context->stopped(m_cookie);
		return;
	}
	common::surface *surf = get_surface();
	if(!surf) {
		lib::logger::get_logger()->show("No surface [%s]",
			url.get_url().c_str());
		m_context->stopped(m_cookie);
		return;
	}
	dx_window *dxwindow = static_cast<dx_window*>(surf->get_gui_window());
	viewport *v = dxwindow->get_viewport();
	m_image = new image_renderer(url, src, v);
	if(!m_image) {
		// Notify scheduler
		m_context->stopped(m_cookie);
		return;
	}
	
	// Does the renderer have all the resources to play?
	if(!m_image->can_play()) {
		// Notify scheduler
		m_context->stopped(m_cookie);
		return;
	}
	
	// Has this been activated
	if(m_activated) {
		// repeat
		m_dest->need_redraw();
		return;	
	}
	m_context->started(m_cookie);

	// Activate this renderer.
	// Add this renderer to the display list of the region
	m_dest->show(this);
	m_dest->need_events(m_wantclicks);
	m_activated = true;
		
	// Request a redraw
	// Currently already done by show()
	// m_dest->need_redraw();

	// Notify scheduler that we're done playing
	m_context->stopped(m_cookie);
}

void gui::dx::dx_img_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("dx_img_renderer::stop(0x%x)", this);
	delete m_image;
	m_image = 0;
	m_dest->renderer_done(this);
	m_activated = false;
	m_dxplayer->stopped(this);
	m_dest->need_redraw();
}

void gui::dx::dx_img_renderer::user_event(const lib::point& pt, int what) {
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
}

void gui::dx::dx_img_renderer::redraw(const lib::rect& dirty, common::gui_window *window) {
	// Get the top-level surface
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();
	if(!v) {
		AM_DBG lib::logger::get_logger()->debug("dx_img_renderer::redraw NOT: no viewport %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());
		return;
	}
	
	if(!m_image || !m_image->can_play()) {
		// No bits available
		AM_DBG lib::logger::get_logger()->debug("dx_img_renderer::redraw NOT: no image or cannot play %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());
		return;
	}

	lib::rect img_rect1;
	lib::rect img_reg_rc;
	lib::size srcsize = m_image->get_size();

	// This code could be neater: it could share quite a bit with the
	// code below (for non-tiled images). Also, support for tiled images
	// is specifically geared toward background images: stuff like the
	// dirty region and transitions are ignored.
	// Also, it knows that the node and the region we're painting to are
	// really the same node.
	if (m_node->get_attribute("backgroundImage") &&  m_dest->is_tiled()) {
		AM_DBG lib::logger::get_logger()->debug("dx_img_renderer.redraw: drawing tiled image");
		img_reg_rc = m_dest->get_rect();
		img_reg_rc.translate(m_dest->get_global_topleft());
		common::tile_positions tiles = m_dest->get_tiles(srcsize, img_reg_rc);
		common::tile_positions::iterator it;
		for(it=tiles.begin(); it!=tiles.end(); it++) {
			img_rect1 = (*it).first;
			img_reg_rc = (*it).second;
			v->draw(m_image->get_ddsurf(), img_rect1, img_reg_rc, m_image->is_transparent());
		}

		if (m_erase_never) m_dest->keep_as_background();
		return;
	}
#ifdef WITH_SMIL30
	lib::rect croprect = m_dest->get_crop_rect(srcsize);
	img_reg_rc = m_dest->get_fit_rect(croprect, srcsize, &img_rect1, m_alignment);
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
		}
	}
#else
	// Get fit rectangles
	img_reg_rc = m_dest->get_fit_rect(srcsize, &img_rect1, m_alignment);
#endif
	// Use one type of rect to do op
	lib::rect img_rect(img_rect1);
	
	// A complete repaint would be:  
	// {img, img_rect } -> img_reg_rc
	
	// We have to paint only the intersection.
	// Otherwise we will override upper layers 
	lib::rect img_reg_rc_dirty = img_reg_rc & dirty;
	if(img_reg_rc_dirty.empty()) {
		// this renderer has no pixels for the dirty rect
		AM_DBG lib::logger::get_logger()->debug("dx_img_renderer::redraw NOT: empty dirty region %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());
		return;
	}	
	
	// Find the part of the image that is mapped to img_reg_rc_dirty
	lib::rect img_rect_dirty = reverse_transform(&img_reg_rc_dirty, 
		&img_rect, &img_reg_rc);
		
	// Translate img_reg_rc_dirty to viewport coordinates 
	lib::point topleft = m_dest->get_global_topleft();
	img_reg_rc_dirty.translate(topleft);
	
	// keep rect for debug messages
	m_msg_rect |= img_reg_rc_dirty;
	
	// Finally blit img_rect_dirty to img_reg_rc_dirty
	AM_DBG lib::logger::get_logger()->debug("dx_img_renderer::redraw %0x %s ", m_dest, m_node->get_url("src").get_url().c_str());
	
	dx_transition *tr = get_transition();
	if (tr && tr->is_fullscreen()) {
		v->set_fullscreen_transition(tr);
		tr = NULL;
	}

	if(tr && tr->is_outtrans()) {
		// First draw the background color, if applicable
		const common::region_info *ri = m_dest->get_info();
		if(ri)
			v->clear(img_reg_rc_dirty,ri->get_bgcolor(), ri->get_bgopacity());
		// Next, take a snapshot of the relevant pixels as they are now, before we draw the image
		lib::size image_size = m_image->get_size();
		IDirectDrawSurface *bgimage = v->create_surface(image_size);
		lib::rect dirty_screen = img_rect_dirty;
		dirty_screen.translate(topleft);
		RECT bgrect_image, bgrect_screen;
		set_rect(img_rect_dirty, &bgrect_image);
		set_rect(dirty_screen, &bgrect_screen);
#ifdef DDBLT_WAIT
#define WAITFLAG DDBLT_WAIT
#else
#define WAITFLAG DDBLT_WAITNOTBUSY
#endif
		bgimage->Blt(&bgrect_image, v->get_surface(), &bgrect_screen, WAITFLAG, NULL);
		// Then draw the image
		v->draw(m_image->get_ddsurf(), img_rect_dirty, img_reg_rc_dirty, m_image->is_transparent(), (dx_transition*)0);
		// And finally transition in the background bits saved previously
		v->draw(bgimage, img_rect_dirty, img_reg_rc_dirty, false, tr);
		bgimage->Release();
	} else {
#ifdef	WITH_SMIL30
		if (alpha_chroma != 1.0) {
			IDirectDrawSurface* screen_ddsurf = v->get_surface(); 
			IDirectDrawSurface* image_ddsurf = m_image->get_ddsurf();
			lib::rect rct0 (lib::point(0, 0), img_rect_dirty.size());
			v->blend_surface(img_rect_dirty, image_ddsurf,
							 rct0, m_image->is_transparent(), 
							 alpha_chroma, alpha_media,
							 chroma_low, chroma_high);

//XX		lib::rect rct0 (lib::point(0, 0), lib::size(N_W, N_H));
//XX		qt_image_blend (screen_img, dstrect, scaledimage, rct0, 
//XX				alpha_chroma, alpha_media,
//XX				chroma_low, chroma_high);
//XX			paint.drawImage(D_L, D_T, screen_img, D_L, D_T, D_W, D_H);
		} else {
			v->draw(m_image->get_ddsurf(), img_rect_dirty, img_reg_rc_dirty, m_image->is_transparent(), tr);
		}
#else //WITH_SMIL30		
		v->draw(m_image->get_ddsurf(), img_rect_dirty, img_reg_rc_dirty, m_image->is_transparent(), tr);
#endif//WITH_SMIL30
	}
	if (m_erase_never) m_dest->keep_as_background();
}



