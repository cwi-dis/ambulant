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
#include "ambulant/gui/dx/jpg_decoder.h"
#include "ambulant/gui/dx/gif_decoder.h"
#include "ambulant/gui/dx/png_decoder.h"
#include "ambulant/gui/dx/bmp_decoder.h"

#include "ambulant/common/region.h"
#include "ambulant/lib/layout.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/logger.h"

#include "jpeglib.h"

using namespace ambulant;

////////////////////////
// img_decoder factory function

typedef gui::dx::img_decoder<net::active_datasource, lib::color_trible> img_decoder_class;

static img_decoder_class*
create_img_decoder(net::active_datasource *src, HDC hdc) {
	typedef gui::dx::jpg_decoder<net::active_datasource, lib::color_trible> jpg_decoder_class;
	typedef gui::dx::gif_decoder<net::active_datasource, lib::color_trible> gif_decoder_class;
	typedef gui::dx::png_decoder<net::active_datasource, lib::color_trible> png_decoder_class;
	typedef gui::dx::bmp_decoder<net::active_datasource, lib::color_trible> bmp_decoder_class;
	
	img_decoder_class* decoder = 0;
	
	decoder = new jpg_decoder_class(src, hdc);
	if(decoder->can_decode()) return decoder;
	delete decoder;
	
	decoder = new gif_decoder_class(src, hdc);
	if(decoder->can_decode()) return decoder;
	delete decoder;
	
	decoder = new png_decoder_class(src, hdc);
	if(decoder->can_decode()) return decoder;
	delete decoder;
	
	decoder = new bmp_decoder_class(src, hdc);
	if(decoder->can_decode()) return decoder;
	delete decoder;
	
	return 0;
}

////////////////////////
//

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
	
	const lib::abstract_smil_region_info *ri = m_dest->get_info();
	
	// Create a dx-region
	viewport *v = get_viewport();
	lib::screen_rect<int> rc = m_dest->get_rect();
	lib::point pt = m_dest->get_global_topleft();
	rc.translate(pt);
	m_region = v->create_region(rc, v->get_rc(), ri?ri->get_zindex():0);
	
	m_region->set_rendering_surface(m_dest);
	m_region->set_rendering_info(ri);
	m_region->set_background(ri?ri->get_bgcolor():CLR_INVALID);
	m_region->clear();
	
	if(!m_src->exists()) {
		m_dest->show(this);
		lib::logger::get_logger()->error("The location specified for the data source does not exist.");
		stopped_callback();
		return;
	}
	typedef lib::no_arg_callback<dx_img_renderer> callback_t;
	lib::event *e = new callback_t(this, &dx_img_renderer::readdone);
	m_src->start(m_event_processor, e);
}

void gui::dx::dx_img_renderer::readdone() {
	lib::logger::get_logger()->trace("dx_img_renderer.readdone(0x%x, size=%d)", (void *)this, m_src->size());
	
	// Prepare dx-region's pixel map
	HDC hdc = ::GetDC(NULL);
	img_decoder_class* decoder = create_img_decoder(m_src, hdc);
	::DeleteDC(hdc);
	if(decoder) {
		dib_surface<lib::color_trible> *ds = decoder->decode();
		if(ds) {
			m_region->set_bmp(ds->get_handle(), 
				(int)ds->get_pixmap()->get_width(), (int)ds->get_pixmap()->get_height(),
				decoder->is_transparent(), decoder->get_transparent_color());
		}
		delete decoder;
		delete ds;
	}
	m_dest->need_redraw();
	stopped_callback();
}


void gui::dx::dx_img_renderer::stop() {
	viewport *v = get_viewport();
	if(v && m_region) {
		v->remove_region(m_region);
		m_region = 0;
		v->redraw();
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
 

