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
	lib::event_processor* evp,
	common::gui_window *window) 
:	common::renderer_playable(context, cookie, node, evp), 
	m_rgn(0) {
	AM_DBG lib::logger::get_logger()->debug("dx_area::ctr(0x%x)", this);
}

gui::dx::dx_area::~dx_area() {
	delete m_rgn;
}

void gui::dx::dx_area::start(double t) {		
	if(m_activated) return;	
	lib::screen_rect<int> rrc = m_dest->get_rect();
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
		lib::screen_rect<int> rc;
		rc.set_coord(l, t, l+w, t+h);
		m_rgn = new dx_gui_region(rc);
	}
	m_dest->show(this);
	m_dest->need_events(m_wantclicks);
	m_activated = true;
}

void gui::dx::dx_area::stop() {
	m_dest->renderer_done(this);
	m_activated = false;
	if(m_rgn) {
		delete m_rgn;
		m_rgn = 0;
	}
}

void gui::dx::dx_area::redraw(const lib::screen_rect<int> &dirty, 
	common::gui_window *window) {
	if(!m_rgn) return;
	AM_DBG {
		dx_window *dxwindow = static_cast<dx_window*>(window);
		viewport *v = dxwindow->get_viewport();
		if(!v) return;
		
		lib::screen_rect<int> reg_rc = m_rgn->get_bounding_box();
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
