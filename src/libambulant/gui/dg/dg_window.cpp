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

#include "ambulant/gui/dg/dg_window.h"
#include "ambulant/gui/dg/dg_viewport.h"
#include "ambulant/common/region.h"

#include "ambulant/lib/logger.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dg::dg_window::dg_window(const std::string& name, 
  	lib::size bounds,
  	region *rgn,
  	common::window_factory *wf,
  	viewport* v)
:	common::gui_window(rgn),
	m_rgn(rgn),
	m_name(name),
	m_viewrc(point(0, 0), size(bounds.w, bounds.h)),
	m_wf(wf),
	m_viewport(v) {
}

gui::dg::dg_window::~dg_window() {
	AM_DBG lib::logger::get_logger()->debug("~dg_window()");
	m_wf->window_done(m_name);
}
  		
void gui::dg::dg_window::need_redraw(const lib::rect &r) {
	lib::rect rc = r;
	rc &= m_viewrc;
	m_rgn->redraw(rc, this);
	m_viewport->redraw(rc);
}

void gui::dg::dg_window::need_redraw() {
	m_rgn->redraw(m_viewrc, this);
	m_viewport->redraw();
}

