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

#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_smiltext.h"
#include "ambulant/gui/dx/dx_transition.h"

#include "ambulant/common/region_info.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/memfile.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/smil2/params.h"
#include "ambulant/common/factory.h"

#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

gui::dx::dx_smiltext_renderer::dx_smiltext_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::factories* factory,
	dx_playables_context *dxplayer)
:   dx_renderer_playable(context, cookie, node, evp, dxplayer),
//X	m_text_storage(NULL),
	m_text(0),
	m_x(0),
	m_y(0),
	m_df(factory->get_datasource_factory()),
	m_engine(smil2::smiltext_engine(node, evp, this, true)),
	m_params(m_engine.get_params())
{
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer(0x%x)", this);
	m_text_storage = "";
//X	m_text_size = 1;
}
void gui::dx::dx_smiltext_renderer::set_surface(common::surface *dest) {
	m_dest = dest;
	
	lib::rect rc = dest->get_rect();
	lib::size bounds(rc.width(), rc.height());
	net::url url = m_node->get_url("src");
	dx_window *dxwindow = static_cast<dx_window*>(m_dest->get_gui_window());
	viewport *v = dxwindow->get_viewport();
	m_text = new text_renderer(url, bounds, v); //XXX
#ifdef JUNK
	// Pass <param> settings, if applicable
	smil2::params *params = smil2::params::for_node(m_node);
	if (params) {
		const char *fontname = params->get_str("font-family");
//		const char *fontstyle = params->get_str("font-style");
		lib::color_t smiltext_color = params->get_color("color", 0);
		float fontsize = params->get_float("font-size", 0.0);
		if (fontname) {
			m_text->set_text_font(fontname);
		}
		if (fontsize) {
			m_text->set_text_size(fontsize);
		}
		if (smiltext_color) {
			m_text->set_text_color(smiltext_color);
		}
		delete params;
	}

	m_text->open(m_df);
#endif/*JUNK*/
}

gui::dx::dx_smiltext_renderer::~dx_smiltext_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~dx_smiltext_renderer(0x%x)", this);
}

void gui::dx::dx_smiltext_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer::start(0x%x)", this);
		
	m_epoch = m_event_processor->get_timer()->elapsed();
	m_engine.start(t);
	renderer_playable::start(t);
}

void gui::dx::dx_smiltext_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer::stop(0x%x)", this);
	delete m_text;
	m_text = 0;
	m_dest->renderer_done(this);
	m_activated = false;
	m_dxplayer->stopped(this);
}

void gui::dx::dx_smiltext_renderer::smiltext_changed() {
	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer::smiletext_changed(0x%x)", this);
	m_lock.enter();
	if (m_engine.is_changed()) {
		lib::xml_string data;
		smil2::smiltext_runs::const_iterator i;
		if (m_engine.is_cleared()) {
			// Completely new text. Clear our copy and render everything.
			m_text_storage = "";
			i = m_engine.begin();
		} else {
			// Only additions. Don't clear and only render the new stuff.
			i = m_engine.newbegin();
		}
		while (i != m_engine.end()) {
			AM_DBG lib::logger::get_logger()->debug("dx_smiltext_changed(): command=%d",(*i).m_command);
			// Add the new characters
			const char* newdata;
			if ((*i).m_command == smil2::stc_break)
				newdata = "\n";
			else if ((*i).m_command == smil2::stc_data) {
				newdata = (*i).m_data.c_str();
				AM_DBG lib::logger::get_logger()->debug("dx_smiltext_changed(): %s", newdata);
			} else {
				AM_DBG lib::logger::get_logger()->debug("smiltext_engine produced unknown command 0x%x", (*i).m_command);
				newdata = "";
			}
			m_text_storage += newdata;
			m_text_storage += ' ';
			// Set font, colot and attributes
			m_text->set_text_font((*i).m_font_family);
			m_text->set_text_size((*i).m_font_size);
			if (!(*i).m_transparent) {
				m_text->set_text_color((*i).m_color);
			}
			if (!(*i).m_bg_transparent) {
				m_text->set_text_bgcolor((*i).m_bg_color);
			}		
			if (m_text && m_text_storage != "") {
				m_text->set_text_data(m_text_storage.data(), m_text_storage.length());
				m_text_storage = "";
				UINT uFormat = DT_NOPREFIX|DT_WORDBREAK;
				switch ((*i).m_align) {
					case smil2::sta_left:
					case smil2::sta_start:
						uFormat |= DT_LEFT;
						break;
					case smil2::sta_end:
					case smil2::sta_right:
						uFormat |= DT_RIGHT;
						break;
					case smil2::sta_center:
						uFormat |= DT_CENTER;
						break;
				}
				switch ((*i).m_direction) {
					case smil2::std_rtl:
						uFormat |= DT_RTLREADING;
						switch ((*i).m_align) { // correction of alignment
							case smil2::sta_start:
								uFormat &= ~DT_LEFT;
								uFormat |= DT_RIGHT;
								break;
							case smil2::sta_end:
								uFormat &= ~DT_RIGHT;
								uFormat |= DT_LEFT;
								break;
							default:
								break;
							}
						break;
					default:
						break;
				}
				SIZE sz = m_text->render(m_x, m_y, uFormat);
				m_x += sz.cx;
				m_y += sz.cy;
			}
			i++;
		}//while
		m_engine.done();
	}
	m_lock.leave();
	m_dest->need_redraw();
}

void gui::dx::dx_smiltext_renderer::user_event(const lib::point& pt, int what) {
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
}

void gui::dx::dx_smiltext_renderer::redraw(const lib::rect& dirty, common::gui_window *window) {
	// Get the top-level surface
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();
	if(!v) return;
	
	if(!m_text || !m_text->can_play()) {
//X	if(!m_text) {
		// No bits available
//KB	AM_DBG lib::logger::get_logger()->debug("dx_smiltext_renderer::redraw with no text");
		return;
	}
	lib::rect smiltext_rc = dirty;
	lib::rect reg_rc = dirty;
	
	// Translate img_reg_rc_dirty to viewport coordinates 
	lib::point pt = m_dest->get_global_topleft();
	reg_rc.translate(pt);
		
	dx_transition *tr = get_transition();
	if (tr && tr->is_fullscreen()) {
		v->set_fullscreen_transition(tr);
		tr = NULL;
	}
		
	// Finally blit img_rect_dirty to img_reg_rc_dirty
	v->draw(m_text->get_ddsurf(), smiltext_rc, reg_rc, true, tr);

	if (m_erase_never) m_dest->keep_as_background();
}

 

