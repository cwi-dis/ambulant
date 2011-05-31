// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2011 Stichting CWI, 
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

#include "ambulant/gui/dx/dx_text.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_text_renderer.h"
#include "ambulant/gui/dx/dx_transition.h"
#include "ambulant/common/region_info.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/string_util.h"
#include "ambulant/smil2/params.h"
#include "ambulant/common/factory.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

extern const char dx_text_playable_tag[] = "text";
extern const char dx_text_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererDirectX");
extern const char dx_text_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererDirectXText");
extern const char dx_text_playable_renderer_uri3[] = AM_SYSTEM_COMPONENT("RendererText");

common::playable_factory *
gui::dx::create_dx_text_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirectX"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirectXText"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererText"), true);
	return new common::single_playable_factory<
		gui::dx::dx_text_renderer,
		dx_text_playable_tag,
		dx_text_playable_renderer_uri,
		dx_text_playable_renderer_uri2,
		dx_text_playable_renderer_uri3 >(factory, mdp);
}

gui::dx::dx_text_renderer::dx_text_renderer(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::factories* factory,
	common::playable_factory_machdep *dxplayer)
:	dx_renderer_playable(context, cookie, node, evp, factory, dynamic_cast<dx_playables_context*>(dxplayer)),
	m_text(0),
	m_df(factory->get_datasource_factory())
{
	AM_DBG lib::logger::get_logger()->debug("dx_text_renderer(0x%x)", this);
}

void gui::dx::dx_text_renderer::set_surface(common::surface *dest) {
	m_dest = dest;

	lib::rect rc = dest->get_rect();
	lib::size bounds(rc.width(), rc.height());
	net::url url = m_node->get_url("src");
	dx_window *dxwindow = static_cast<dx_window*>(m_dest->get_gui_window());
	viewport *v = dxwindow->get_viewport();

	m_text = new text_renderer(url, bounds, v);

	// Pass <param> settings, if applicable
	smil2::params *params = smil2::params::for_node(m_node);
	if (params) {
		const char *fontname = params->get_str("font-family");
//		const char *fontstyle = params->get_str("font-style");
		lib::color_t text_color = params->get_color("color", GetSysColor(COLOR_WINDOWTEXT));
		float fontsize = params->get_float("font-size", 0.0);
		if (fontname) {
			m_text->set_text_font(fontname);
		}
		if (fontsize) {
			m_text->set_text_size(fontsize);
		}
		m_text->set_text_color(text_color);
		delete params;
	}

	m_text->open(m_df);
	m_text->render();
}

gui::dx::dx_text_renderer::~dx_text_renderer() {
	AM_DBG lib::logger::get_logger()->debug("~dx_text_renderer(0x%x)", this);
}

void gui::dx::dx_text_renderer::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("dx_text_renderer::start(0x%x)", this);

	if(!m_text) {
		// Notify scheduler
		m_context->stopped(m_cookie);
		return;
	}

	// Has this been activated
	if(m_activated) {
		// repeat
		m_dest->need_redraw();
		return;
	}
	m_context->started(m_cookie);
	// Activate this renderer.
	// Add this renderer to the display list of the region
	m_dest->show(this);
	m_dest->need_events(m_wantclicks);
	m_activated = true;

	// Request a redraw
	// Currently already done by show()
	// m_dest->need_redraw();

	// Notify scheduler that we're done playing
	m_context->stopped(m_cookie);
}

bool gui::dx::dx_text_renderer::stop() {
	AM_DBG lib::logger::get_logger()->debug("dx_text_renderer::stop(0x%x)", this);
	delete m_text;
	m_text = 0;
	if (m_dest) m_dest->renderer_done(this);
	m_dest = NULL;
	m_activated = false;
	m_dxplayer->stopped(this);
	return true;
}

bool gui::dx::dx_text_renderer::user_event(const lib::point& pt, int what) {
	if (!user_event_sensitive(pt)) return false;
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
	return true;
}

void gui::dx::dx_text_renderer::redraw(const lib::rect& dirty, common::gui_window *window) {
	// Get the top-level surface
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();
	if(!v) return;

	if(!m_text || !m_text->can_play()) {
		// No bits available
		AM_DBG lib::logger::get_logger()->debug("dx_text_renderer::redraw with no text");
		return;
	}

	lib::rect text_rc = dirty;
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
	v->draw(m_text->get_ddsurf(), text_rc, reg_rc, true, tr);

	if (m_erase_never) m_dest->keep_as_background();
}



