/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
 */

#include "ambulant/gui/dx/dx_img.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_image_renderer.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/memfile.h"
#include "ambulant/lib/logger.h"

#include "ambulant/common/region_info.h"

#include <math.h>

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
	common::abstract_window *window)
:   common::renderer_playable(context, cookie, node, evp),
	m_image(0), m_window(window) {
	
	AM_DBG lib::logger::get_logger()->trace("dx_img_renderer::ctr(0x%x)", this);
	std::string url = m_node->get_url("src");
	if(!window) {
		lib::logger::get_logger()->show("get_window() failed. [%s]",
			url.c_str());
		return;
	}
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();
	if(lib::memfile::exists(url)) {
		m_image = new image_renderer(m_node->get_url("src"), v);
	} else {
		lib::logger::get_logger()->show("The location specified for the data source does not exist. [%s]",
			url.c_str());
	}
}

gui::dx::dx_img_renderer::~dx_img_renderer() {
	AM_DBG lib::logger::get_logger()->trace("dx_img_renderer::dtr(0x%x)", this);
	delete m_image;
}


void gui::dx::dx_img_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->trace("dx_img_renderer::start(0x%x)", this);
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
	
	// Activate this renderer.
	// Add this renderer to the display list of the region
	m_dest->show(this);
	m_dest->need_events(m_wantclicks);
	m_activated = true;
		
	// Request a redraw
	// Currently already done by show()
	// m_dest->need_redraw();
}

void gui::dx::dx_img_renderer::stop() {
	AM_DBG lib::logger::get_logger()->trace("dx_img_renderer::stop(0x%x)", this);
	delete m_image;
	m_image = 0;
	m_dest->renderer_done();
	m_activated = false;
	
	// show debug message 'stopped'
	AM_DBG {
		dx_window *dxwindow = static_cast<dx_window*>(m_window);
		viewport *v = dxwindow->get_viewport();
		if(v) {
			if(!m_msg_rect.empty()) {
				v->draw(text_str("STOPPED"), m_msg_rect, lib::to_color("red"));
				v->redraw(m_msg_rect);
			}
		}
	}
}

void gui::dx::dx_img_renderer::user_event(const lib::point& pt, int what) {
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
}

void gui::dx::dx_img_renderer::redraw(const lib::screen_rect<int>& dirty, common::abstract_window *window) {
	// Get the top-level surface
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();
	if(!v) return;
	
	if(!m_image || !m_image->can_play()) {
		// No bits available
		return;
	}
	
	// Get fit rectangles
	lib::rect img_rect1;
	lib::screen_rect<int> img_reg_rc = m_dest->get_fit_rect(m_image->get_size(), &img_rect1);
	
	// Use one type of rect to do op
	lib::screen_rect<int> img_rect(img_rect1);
	
	// A complete repaint would be:  
	// {img, img_rect } -> img_reg_rc
	
	// We have to paint only the intersection.
	// Otherwise we will override upper layers 
	lib::screen_rect<int> img_reg_rc_dirty = img_reg_rc & dirty;
	if(img_reg_rc_dirty.empty()) {
		// this renderer has no pixels for the dirty rect
		return;
	}	
	
	// Find the part of the image that is mapped to img_reg_rc_dirty
	lib::screen_rect<int> img_rect_dirty = reverse_transform(&img_reg_rc_dirty, 
		&img_rect, &img_reg_rc);
		
	// Translate img_reg_rc_dirty to viewport coordinates 
	lib::point pt = m_dest->get_global_topleft();
	img_reg_rc_dirty.translate(pt);
	
	// keep rect for debug messages
	m_msg_rect |= img_reg_rc_dirty;
	
	// Finally blit img_rect_dirty to img_reg_rc_dirty
	//AM_DBG lib::logger::get_logger()->trace("dx_img_renderer::redraw %0x %s ", m_dest, m_node->get_url("src").c_str());
	v->draw(m_image->get_ddsurf(), img_rect_dirty, img_reg_rc_dirty, m_image->is_transparent());
}



