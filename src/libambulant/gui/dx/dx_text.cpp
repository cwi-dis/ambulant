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
#include "ambulant/lib/node.h"

using namespace ambulant;

gui::dx::dx_text_renderer::dx_text_renderer(
	lib::event_processor *const evp,
	net::passive_datasource *src,
	lib::passive_region *const dest,
	const lib::node *node)
:   lib::active_renderer(evp, src, dest, node) {
}

gui::dx::dx_text_renderer::~dx_text_renderer() {
}

void gui::dx::dx_text_renderer::start(lib::event *playdone) {
	if(!m_node || !m_src) abort();
	m_playdone = playdone;
	if(!m_src->exists()) {
		lib::logger::get_logger()->error("The location specified for the data source does not exist.");
		m_event_processor->add_event(playdone, 0, lib::event_processor::low);
		return;
	}
	// Create a dx-region
	viewport *v = get_viewport();
	m_region = v->create_region(m_dest->get_rect_outer(), m_dest->get_parent()->get_rect_outer());
	
	// Prepare dx-region's pixel map
	m_region->set_background("teal");
	m_region->clear();
	
	m_dest->show(this);
	m_src->start(m_event_processor, m_readdone);
}

void gui::dx::dx_text_renderer::readdone() {
	lib::logger::get_logger()->trace("dx_text_renderer.readdone(0x%x, size=%d)", (void *)this, m_src->size());
	lib::logger::ostream os = lib::logger::get_logger()->trace_stream();
	
	// Prepare dx-region's pixel map
	net::databuffer& db = m_src->get_databuffer();
	m_region->set_text((const char*) db.data(), int(db.size()));
	
	m_dest->need_redraw();
	if(m_playdone)
		m_event_processor->add_event(m_playdone, 0, lib::event_processor::low);
}

void gui::dx::dx_text_renderer::stop() {
	viewport *v = get_viewport();
	v->remove_region(m_region);
	m_region = 0;
	lib::active_renderer::stop();
}

void gui::dx::dx_text_renderer::redraw(const lib::screen_rect<int> &dirty, 
	lib::passive_window *window, const lib::point &window_topleft) {
	viewport *v = get_viewport(window);
	v->redraw();
}

gui::dx::viewport* gui::dx::dx_text_renderer::get_viewport() {
	const lib::passive_region *top = 0;
	const lib::passive_region *p = m_dest->get_parent();
	while(p != 0) {
		top = p;
		p = p->get_parent();
	}
	dx_window *dxwindow = (dx_window *) top;
	return dxwindow->get_viewport();
}

gui::dx::viewport* gui::dx::dx_text_renderer::get_viewport(lib::passive_window *window) {
	dx_window *dxwindow = (dx_window *) window;
	return dxwindow->get_viewport();
}
 

