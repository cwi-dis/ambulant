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

#include "ambulant/gui/dx/dx_brush.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"

#include "ambulant/common/region_info.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/logger.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dx::dx_brush::dx_brush(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::gui_window *window,
	dx_playables_context *dxplayer)
:   dx_renderer_playable(context, cookie, node, evp, window, dxplayer),
	m_color(0) {
	AM_DBG lib::logger::get_logger()->debug("dx_brush::dx_brush(0x%x)", this);
}

gui::dx::dx_brush::~dx_brush() {
	AM_DBG lib::logger::get_logger()->debug("~dx_brush()");
}

void gui::dx::dx_brush::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("dx_brush::start(0x%x)", this);
	
	// Has this been activated
	if(m_activated) {
		// repeat
		return;	
	}
	
	const char *color_attr = m_node->get_attribute("color");
	if(color_attr && lib::is_color(color_attr))
		m_color = lib::to_color(color_attr);
	else {
		const common::region_info *ri = m_dest->get_info();
		m_color = ri?ri->get_bgcolor():0;
	}
	
	// Activate this renderer.
	// Add this renderer to the display list of the region
	m_dest->show(this);
	m_dest->need_events(m_wantclicks);
	m_activated = true;
		
	// Request a redraw
	m_dest->need_redraw();
}


void gui::dx::dx_brush::stop() {
	AM_DBG lib::logger::get_logger()->debug("dx_brush::stop(0x%x)", this);
	m_dest->renderer_done(this);
	m_activated = false;
	m_dxplayer->stopped(this);
}

void gui::dx::dx_brush::user_event(const lib::point& pt, int what) {
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
}

void gui::dx::dx_brush::redraw(const lib::screen_rect<int> &dirty, common::gui_window *window) {
	AM_DBG lib::logger::get_logger()->debug("dx_brush::redraw(): %s", repr(dirty).c_str());
	// Get the top-level surface
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();
	if(!v) return;
	
	// Draw the pixels of this renderer to the surface specified by m_dest.
	lib::screen_rect<int> rc = dirty;
	lib::point pt = m_dest->get_global_topleft();
	rc.translate(pt);
	
	dx_transition *tr = 0;
	if(m_transitioning) {
		tr = m_dxplayer->get_transition(this);
		m_transitioning = tr?true:false;
	}	
	v->clear(rc, m_color, tr);
}
 

