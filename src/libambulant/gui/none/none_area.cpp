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

#include "ambulant/gui/none/none_area.h"
#include "ambulant/lib/logger.h"
#include "ambulant/common/region_dim.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace gui::none;

none_area_renderer::~none_area_renderer()
{
	if (m_rgn) delete m_rgn;
}

void
none_area_renderer::start(double starttime) {
	AM_DBG lib::logger::get_logger()->debug("none_area_renderer(0x%x)::start()", (void*)this);
	if(m_activated) return;
	lib::rect rrc = m_dest->get_rect();
	AM_DBG lib::logger::get_logger()->debug("none_area_renderer::start(%s)",
		repr(rrc).c_str());

	const char *coords = m_node->get_attribute("coords");
	const char *shape = m_node->get_attribute("shape");
	if(!coords || !coords[0]) {
		m_rgn = new lib::rect(m_dest->get_rect());
		AM_DBG lib::logger::get_logger()->debug("none_area_renderer::start: no coords, whole area, lt=(%d, %d) wh=(%d, %d)",
				m_rgn->x, m_rgn->y, m_rgn->w, m_rgn->h);
	} else {
		common::region_dim_spec rds(coords, shape);
		rds.convert(rrc);
		int l = rds.left.absolute()?rds.left.get_as_int():rrc.left();
		int t = rds.top.absolute()?rds.top.get_as_int():rrc.top();
		int w = rds.width.absolute()?rds.width.get_as_int():rrc.width();
		int h = rds.height.absolute()?rds.height.get_as_int():rrc.height();
		AM_DBG lib::logger::get_logger()->debug("none_area_renderer::start: lt=(%d,%d) wh=(%d,%d)", l, t, w, h);
		m_rgn = new lib::rect(lib::point(l, t), lib::size(w, h));
	}
	AM_DBG lib::logger::get_logger()->debug("none_area_renderer::start: wantclicks=%d", m_wantclicks);
	m_dest->need_events(m_wantclicks);
	renderer_playable::start(starttime);
	m_context->started(m_cookie);
	m_context->stopped(m_cookie);
}

bool
none_area_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("none_area_renderer(0x%x)::stop()", (void*)this);
	renderer_playable::stop();
	if(m_rgn) {
		delete m_rgn;
		m_rgn = 0;
	}
	m_context->stopped(m_cookie);

	return true; // Don't re-use this renderer
}

bool
none_area_renderer::user_event(const lib::point& pt, int what) {
	AM_DBG lib::logger::get_logger()->debug("%s: renderer_playable::user_event((%d,%d), %d) -> %d", m_node->get_sig().c_str(), pt.x, pt.y, what, m_cookie);
	if(!m_rgn) {
		AM_DBG lib::logger::get_logger()->debug("none_area_renderer: no region");
		return false;
	}
	if(!m_rgn->contains(pt)) {
		AM_DBG lib::logger::get_logger()->debug("none_area_renderer: not in our region");
		return false;
	}
	if(what == common::user_event_click) {
		AM_DBG lib::logger::get_logger()->debug("none_area_renderer(0x%x)::user_event((%d,%d), %d): clicked", (void*)this, pt.x, pt.y, what);
		m_context->clicked(m_cookie);
	} else if(what == common::user_event_mouse_over) {
		AM_DBG lib::logger::get_logger()->debug("none_area_renderer(0x%x)::user_event((%d,%d), %d): pointed", (void*)this, pt.x, pt.y, what);
		m_context->pointed(m_cookie);
	}
	return false;
}
