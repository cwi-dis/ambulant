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

#include "ambulant/gui/dx/dx_text.h"
#include "ambulant/gui/dx/dx_gui.h"
#include "ambulant/gui/dx/dx_viewport.h"

#include "ambulant/common/region.h"
#include "ambulant/common/layout.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/memfile.h"

using namespace ambulant;

gui::dx::dx_text_renderer::dx_text_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::abstract_window *window)
:   common::active_renderer(context, cookie, node, evp),
	m_activated(false) { 
	
	lib::logger::get_logger()->trace("dx_text_renderer(0x%x)", this);
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();	
	std::string url = m_node->get_url("src");
	if(!lib::memfile::exists(url)) {
		lib::logger::get_logger()->error("The location specified for the data source does not exist. [%s]",
			m_node->get_url("src").c_str());
		return;
	}
	lib::memfile mf(url);
	mf.read();
	lib::databuffer& db = mf.get_databuffer();
	m_text.assign(db.begin(), db.end());
}

gui::dx::dx_text_renderer::~dx_text_renderer() {
	lib::logger::get_logger()->trace("~dx_text_renderer(0x%x)", this);
}

void gui::dx::dx_text_renderer::start(double t) {
	lib::logger::get_logger()->trace("dx_text_renderer::start(0x%x)", this);
		
	// Has this been activated
	if(m_activated) {
		// repeat
		return;	
	}
	
	// Activate this renderer.
	// Add this renderer to the display list of the region
	m_dest->show(this);
	m_activated = true;
		
	// Request a redraw
	m_dest->need_redraw();
}

void gui::dx::dx_text_renderer::stop() {
	lib::logger::get_logger()->trace("dx_text_renderer::stop(0x%x)", this);
	m_dest->renderer_done();
	m_activated = false;
	m_text = "";
}

void gui::dx::dx_text_renderer::redraw(const lib::screen_rect<int> &dirty, common::abstract_window *window) {
	// Get the top-level surface
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();
	if(!v) return;
	
	// Draw the pixels of this renderer to the surface specified by m_dest.
	lib::screen_rect<int> rc = m_dest->get_rect();
	lib::point pt = m_dest->get_global_topleft();
	rc.translate(pt);
	//const common::region_info *ri = m_dest->get_info();
	if(!m_text.empty()) {
		v->draw(m_text, rc);
		v->redraw();
	}
}

 

