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

#include "ambulant/gui/dx/dx_img.h"

#include "ambulant/gui/dx/dx_gui.h"
#include "ambulant/gui/dx/dx_viewport.h"

#include "ambulant/common/region.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/logger.h"

#include "jpeglib.h"

using namespace ambulant;

gui::dx::dx_img_renderer::dx_img_renderer(
	lib::active_playable_events *context,
	lib::active_playable_events::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	net::passive_datasource *src,
	lib::abstract_rendering_surface *const dest,
	lib::abstract_window *window)
:   lib::active_renderer(context, cookie, node, evp, src, dest),
	m_window(window), m_region(0) { 
}

gui::dx::dx_img_renderer::~dx_img_renderer() {
	lib::logger::get_logger()->trace("~dx_img_renderer()");
}

void gui::dx::dx_img_renderer::start(double t) {
	// On repeat this will be called again
	if(m_region != 0) return;
	
	if(!m_node || !m_src) abort();
	
	// Create a dx-region
	viewport *v = get_viewport();
	const lib::screen_rect<int>& rc = m_dest->get_rect();
	lib::screen_rect<int> rcv(rc);
	rcv.translate(m_dest->get_global_topleft());
	m_region = v->create_region(rcv, rc);
	
	// Prepare dx-region's pixel map bgd
	m_region->set_background("silver");
	m_region->clear();
	
	m_dest->show(this);
	if(!m_src->exists()) {
		lib::logger::get_logger()->error("The location specified for the data source does not exist.");
		stopped_callback();
		return;
	}
	m_src->start(m_event_processor, m_readdone);
}

void gui::dx::dx_img_renderer::readdone() {
	lib::logger::get_logger()->trace("dx_img_renderer.readdone(0x%x, size=%d)", (void *)this, m_src->size());
	
	// Prepare dx-region's pixel map
	typedef jpg_decoder<net::active_datasource, lib::color_trible> decoder_class;
	
	HDC hdc = ::GetDC(NULL);
	decoder_class* decoder = new decoder_class(m_src, hdc);
	if(decoder->can_decode()) {
		dib_surface<lib::color_trible> *ds = decoder->decode();
		m_region->set_bmp(ds->m_hbmp);
	}
	delete decoder;
	::DeleteDC(hdc);
	
	m_dest->need_redraw();
	stopped_callback();
}

void gui::dx::dx_img_renderer::stop() {
	viewport *v = get_viewport();
	if(v && m_region) {
		v->remove_region(m_region);
		m_region = 0;
	}
	lib::active_renderer::stop();
}

void gui::dx::dx_img_renderer::redraw(const lib::screen_rect<int> &dirty, lib::abstract_window *window) {
	viewport *v = get_viewport(window);
	if(v) v->redraw();
}

gui::dx::viewport* gui::dx::dx_img_renderer::get_viewport() {
	return get_viewport(m_window);
}

gui::dx::viewport* gui::dx::dx_img_renderer::get_viewport(lib::abstract_window *window) {
	dx_window *dxwindow = (dx_window *) window;
	return dxwindow->get_viewport();
}
 

