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

#include "ambulant/gui/dg/dg_bgrenderer.h"
#include "ambulant/gui/dg/dg_window.h"
#include "ambulant/gui/dg/dg_viewport.h"

#include "ambulant/common/region_info.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dg::dg_bgrenderer::dg_bgrenderer(const common::region_info *src)
:	common::background_renderer(src),
	m_bg_image(NULL)
{
	AM_DBG lib::logger::get_logger()->debug("new dg_bgrenderer<0x%x>", this);
}
	
gui::dg::dg_bgrenderer::~dg_bgrenderer() {
	AM_DBG lib::logger::get_logger()->debug("~dg_bgrenderer(0x%x)", this);
}
	
void gui::dg::dg_bgrenderer::keep_as_background() {
	AM_DBG lib::logger::get_logger()->debug("dg_bgrenderer::keep_as_background(0x%x)", this);
	dg_window *dgwindow = (dg_window *)m_dst->get_gui_window();
	viewport *v = dgwindow->get_viewport();	
	lib::rect dstrect = m_dst->get_rect();
//	lib::rect srcrect = dstrect;
//	srcrect.translate(m_dst->get_global_topleft());

	if (m_bg_image) delete m_bg_image;
	m_bg_image = v->create_surface(dstrect.width(), dstrect.height());
	if (m_bg_image == NULL) {
		lib::logger::get_logger()->debug("dg_bgrenderer::keep_as_background: Cannot create surface");
		return;
	}
	surface_t *dst = m_bg_image->get_pixmap();
	assert(dst);
	surface_t *src = v->get_surface();
	assert(src);
	dst->blit(src, dstrect, 0, 0, 0);
}
	
void gui::dg::dg_bgrenderer::redraw(const lib::rect &dirty, common::gui_window *window) {
	AM_DBG lib::logger::get_logger()->debug("dg_bgrenderer::redraw(%s)",repr(dirty).c_str());
	dg_window *dgwindow = static_cast<dg_window*>(window);
	viewport *v = dgwindow->get_viewport();	
	if (m_bg_image) {
		lib::rect dstrect_whole = m_dst->get_rect();
		dstrect_whole.translate(m_dst->get_global_topleft());
		AM_DBG lib::logger::get_logger()->debug("dx_bgrenderer::redraw: clear to image");
		v->draw(m_bg_image, dstrect_whole);	
	} else {
		lib::rect rc = dirty;
		lib::point pt = m_dst->get_global_topleft();
		rc.translate(pt);
		if(v && m_src && !m_src->get_transparent()) {
			AM_DBG lib::logger::get_logger()->debug("dx_bgrenderer::redraw: clear to color");
			v->clear(rc, m_src->get_bgcolor());
		}
	}
}

void gui::dg::dg_bgrenderer::highlight(common::gui_window *window)
{
}
