// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2008 Stichting CWI, 
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

#include "ambulant/gui/dg/dg_text.h"
#include "ambulant/gui/dg/dg_viewport.h"
#include "ambulant/gui/dg/dg_window.h"

#include "ambulant/common/region_info.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/document.h"
#include "ambulant/lib/memfile.h"
#include "ambulant/lib/textptr.h"

#include "ambulant/smil2/params.h"

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dg::dg_text_renderer::dg_text_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::factories* factory)
:   common::renderer_playable(context, cookie, node, evp),
	m_fontname(NULL),
	m_fontsize(0),
	m_color(CLR_INVALID),
	m_factory(factory)
{ 
	
	AM_DBG lib::logger::get_logger()->debug("dg_text_renderer(0x%x)", this);
}

void gui::dg::dg_text_renderer::set_surface(common::surface *dest)
{
	m_dest = dest;
	dg_window *dgwindow = static_cast<dg_window*>(m_dest->get_gui_window());
	viewport *v = dgwindow->get_viewport();	
	net::url url = m_node->get_url("src");
	char *data;
	size_t datasize;
	if (!net::read_data_from_url(url, m_factory->get_datasource_factory(), &data, &datasize))
		return;

#ifndef UNICODE
	m_text.assign(data, datasize);
#else
	std::string s(data, datasize);
	m_text = lib::textptr(s.c_str()).c_wstr();
#endif
	if (data) free(data);
	// Pass <param> settings, if applicable
	smil2::params *params = smil2::params::for_node(m_node);
	if (params) {
		m_fontname = params->get_str("font-family");
//		const char *fontstyle = params->get_str("font-style");
		m_color = params->get_color("color", 0);
		m_fontsize = params->get_float("font-size", 0.0);
		delete params;
	}
}

gui::dg::dg_text_renderer::~dg_text_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~dg_text_renderer(0x%x)", this);
}

void gui::dg::dg_text_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("dg_text_renderer::start(0x%x)", this);
		
	// Has this been activated
	if(m_activated) {
		// repeat
		m_dest->need_redraw();
		return;	
	}
	
	// Activate this renderer.
	// Add this renderer to the display list of the region
	m_dest->show(this);
	m_dest->need_events(m_wantclicks);
	m_activated = true;
		
	// Request a redraw
	//m_dest->need_redraw();

	// Notify scheduler that we're done playing
	m_context->stopped(m_cookie);
}

void gui::dg::dg_text_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("dg_text_renderer::stop(0x%x)", this);
	if(!m_activated) return;
	m_text = text_str("");
	m_dest->renderer_done(this);
	m_activated = false;
}

bool gui::dg::dg_text_renderer::user_event(const lib::point& pt, int what) {
	if (!user_event_sensitive(where)) return false;
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
	return true;
}

void gui::dg::dg_text_renderer::redraw(const lib::rect &dirty, common::gui_window *window) {
	// Get the top-level surface
	dg_window *dgwindow = static_cast<dg_window*>(window);
	viewport *v = dgwindow->get_viewport();
	if(!v) return;
	
	// Draw the pixels of this renderer to the surface specified by m_dest.
	lib::rect rc = dirty;
	lib::point pt = m_dest->get_global_topleft();
	rc.translate(pt);
	if(!m_text.empty()) v->draw(m_text, rc, m_color, m_fontname, m_fontsize);

	if (m_erase_never) m_dest->keep_as_background();
}

 

