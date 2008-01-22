// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2008 Stichting CWI, 
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

#include "ambulant/gui/dg/dg_brush.h"
#include "ambulant/gui/dg/dg_viewport.h"
#include "ambulant/gui/dg/dg_window.h"

#include "ambulant/common/region_info.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/logger.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

////////////////////////
//

gui::dg::dg_brush::dg_brush(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp)
:   common::renderer_playable(context, cookie, node, evp),
	m_color(0) { 
	AM_DBG lib::logger::get_logger()->debug("dg_brush::dg_brush(0x%x)", this);
}

gui::dg::dg_brush::~dg_brush() {
	AM_DBG lib::logger::get_logger()->debug("~dg_brush()");
}

void gui::dg::dg_brush::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("dg_brush::start(0x%x)", this);
	
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
		
	m_context->started(m_cookie);
	m_context->stopped(m_cookie);
	// Request a redraw
	// Currently done by show()
	// m_dest->need_redraw();
}


void gui::dg::dg_brush::stop() {
	AM_DBG lib::logger::get_logger()->debug("dg_brush::stop(0x%x)", this);
	m_dest->renderer_done(this);
	m_activated = false;
	m_context->stopped(m_cookie);
}

bool gui::dg::dg_brush::user_event(const lib::point& pt, int what) {
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
	return true;
}

void gui::dg::dg_brush::redraw(const lib::rect &dirty, common::gui_window *window) {
	// Get the top-level surface
	dg_window *dgwindow = static_cast<dg_window*>(window);
	viewport *v = dgwindow->get_viewport();
	if(!v) return;
	
	// Draw the pixels of this renderer to the surface specified by m_dest.
	lib::rect rc = dirty;
	lib::point pt = m_dest->get_global_topleft();
	rc.translate(pt);
	v->clear(rc, m_color);
}
 

