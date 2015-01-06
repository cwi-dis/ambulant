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

#include "ambulant/gui/d2/d2_window.h"
#include "ambulant/gui/d2/d2_player.h"
#include "ambulant/common/region.h"
#include "ambulant/lib/logger.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::d2::d2_window::d2_window(const std::string& name,
	lib::size bounds,
	common::gui_events *handler,
	d2_player *player,
	HWND hwnd)
:	common::gui_window(handler),
	m_handler(handler),
	m_name(name),
	m_hwnd(hwnd),
	m_viewrc(lib::point(0, 0), lib::size(bounds.w, bounds.h)),
	m_player(player),
	m_locked(0),
	m_redraw_rect_valid(false)
{
	//AM_DBG lib::logger::get_logger()->trace_stream()
	//	<< "d2_window(" << name << ", " << bounds << ")" << lib::endl;
}

gui::d2::d2_window::~d2_window() {
	AM_DBG lib::logger::get_logger()->debug("~d2_window()");
	m_player->window_done(m_name);
}

void gui::d2::d2_window::_need_redraw(const lib::rect &r) {
	if(!m_locked) {
		RECT rr = m_player->screen_rect(this, r);
		InvalidateRect(m_hwnd, &rr, 0);
	} else {
		if(!m_redraw_rect_valid) {
			m_redraw_rect = r;
			m_redraw_rect_valid = true;
		} else {
			m_redraw_rect |= r;
		}
	}
}

void gui::d2::d2_window::need_redraw() {
	m_redraw_rect_lock.enter();
	_need_redraw(m_viewrc);
	m_redraw_rect_lock.leave();
}

void gui::d2::d2_window::need_redraw(const lib::rect &r) {
	m_redraw_rect_lock.enter();
	_need_redraw(r);
	m_redraw_rect_lock.leave();
}

void gui::d2::d2_window::redraw(const lib::rect &r) {
	// clip rect to this window since the layout does not do this
	lib::rect rc = r;
	rc &= m_viewrc;
	AM_DBG lib::logger::get_logger()->debug("d2_window::redraw(%d,%d,%d,%d): drawing", rc.left(), rc.top(), rc.width(), rc.height());
	//assert(!m_redraw_rect_valid);
	m_handler->redraw(rc, this);
}

void gui::d2::d2_window::redraw_now() {
	m_redraw_rect_lock.enter();
	int keep_lock = m_locked;
	m_locked = 0;
	m_redraw_rect_valid = false;
	_need_redraw(m_viewrc);
	m_redraw_rect_valid = false;
	m_locked = keep_lock;
	m_redraw_rect_lock.leave();
}

void gui::d2::d2_window::redraw() {
	redraw(m_viewrc);
}

void gui::d2::d2_window::lock_redraw() {
	m_redraw_rect_lock.enter();
	m_locked++;
	AM_DBG ambulant::lib::logger::get_logger()->debug("lock_redraw: count is now %d", m_locked);
	m_redraw_rect_lock.leave();
}

void gui::d2::d2_window::unlock_redraw() {
	m_redraw_rect_lock.enter();
	assert(m_locked > 0);
	m_locked--;
	AM_DBG if(m_locked > 0 && m_redraw_rect_valid) ambulant::lib::logger::get_logger()->debug("unlock_redraw: count is now %d", m_locked);
	if (m_locked == 0 && m_redraw_rect_valid) {
		lib::rect to_redraw = m_redraw_rect;
		m_redraw_rect_valid = false;
		_need_redraw(to_redraw);
	}
	m_redraw_rect_lock.leave();
}
