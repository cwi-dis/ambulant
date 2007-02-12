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

#include "ambulant/gui/dx/dx_area.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_rgn.h"
#include "ambulant/common/region_dim.h"

//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dx::dx_area::dx_area(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp) 
:	common::renderer_playable(context, cookie, node, evp), 
	m_rgn(0) {
	AM_DBG lib::logger::get_logger()->debug("dx_area::ctr(0x%x)", this);
}

gui::dx::dx_area::~dx_area() {
	delete m_rgn;
}

void gui::dx::dx_area::start(double t) {		
	if(m_activated) return;	
	lib::rect rrc = m_dest->get_rect();
	AM_DBG lib::logger::get_logger()->debug("dx_area::start(%s)", 
		repr(rrc).c_str());
	
	const char *coords = m_node->get_attribute("coords");
	const char *shape = m_node->get_attribute("shape");
	if(!coords || !coords[0]) {
		m_rgn = new dx_gui_region(m_dest->get_rect());
	} else {
		common::region_dim_spec rds(coords, shape);
		rds.convert(rrc);
		int l = rds.left.absolute()?rds.left.get_as_int():rrc.left();
		int t = rds.top.absolute()?rds.top.get_as_int():rrc.top();
		int w = rds.width.absolute()?rds.width.get_as_int():rrc.width();
		int h = rds.height.absolute()?rds.height.get_as_int():rrc.height();
		lib::rect rc(lib::point(l, t), lib::size(w, h));
		m_rgn = new dx_gui_region(rc);
	}
	m_dest->show(this);
	m_dest->need_events(m_wantclicks);
	m_activated = true;
	m_context->started(m_cookie);
	// XXXJACK: m_context->stopped(m_cookie);
}

void gui::dx::dx_area::stop() {
	m_dest->renderer_done(this);
	m_activated = false;
	if(m_rgn) {
		delete m_rgn;
		m_rgn = 0;
	}
	m_context->stopped(m_cookie);
}

void gui::dx::dx_area::redraw(const lib::rect &dirty, 
	common::gui_window *window) {
	if(!m_rgn) return;
	AM_DBG {
		dx_window *dxwindow = static_cast<dx_window*>(window);
		viewport *v = dxwindow->get_viewport();
		if(!v) return;
		
		lib::rect reg_rc = m_rgn->get_bounding_box();
		lib::point pt = m_dest->get_global_topleft();
		reg_rc.translate(pt);
		
		v->frame_rect(reg_rc, 0xFF);
	}
}

void gui::dx::dx_area::user_event(const lib::point& pt, int what) {
	if(!m_rgn) return;
	if(m_rgn->contains(pt)) {
		if(what == common::user_event_click)
			m_context->clicked(m_cookie);
		else if(what == common::user_event_mouse_over) {
			m_context->pointed(m_cookie);
		}
	}
}
