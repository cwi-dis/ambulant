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

/*
 * @$Id$
 */

#include "ambulant/gui/dx/dx_brush.h"
#include "ambulant/gui/dx/dx_viewport.h"
#include "ambulant/gui/dx/dx_window.h"
#include "ambulant/gui/dx/dx_transition.h"
#include "ambulant/common/region_info.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/logger.h"
#include "ambulant/smil2/test_attrs.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
extern const char dx_brush_playable_tag[] = "brush";
extern const char dx_brush_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererDirectX");
extern const char dx_brush_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererDirectXBrush");

common::playable_factory *
gui::dx::create_dx_brush_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirectX"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererDirectXBrush"), true);
	return new common::single_playable_factory<
		gui::dx::dx_brush,
		dx_brush_playable_tag,
		dx_brush_playable_renderer_uri,
		dx_brush_playable_renderer_uri2,
		dx_brush_playable_renderer_uri2 >(factory, mdp);
}

gui::dx::dx_brush::dx_brush(
	common::playable_notification *context,
	common::playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp,
	common::factories *fp,
	common::playable_factory_machdep *dxplayer)
:	dx_renderer_playable(context, cookie, node, evp, fp, dynamic_cast<dx_playables_context*>(dxplayer)),
	m_color(0) {
	AM_DBG lib::logger::get_logger()->debug("dx_brush::dx_brush(0x%x)", this);
}

gui::dx::dx_brush::~dx_brush() {
	AM_DBG lib::logger::get_logger()->debug("~dx_brush()");
}

void gui::dx::dx_brush::start(double t) {
	AM_DBG lib::logger::get_logger()->debug("dx_brush::start(0x%x)", this);

	// Has this been activated
	if(m_activated) {
		// repeat
		return;
	}

	const char *color_attr = m_node->get_attribute("color");
	if(color_attr && lib::is_color(color_attr))
		m_color = lib::to_color(color_attr);
	else {
		const common::region_info *ri = m_dest->get_info();
		m_color = ri?ri->get_bgcolor():0;
	}

	// Activate this renderer.
	// Add this renderer to the display list of the region
	m_dest->show(this);
	m_dest->need_events(m_wantclicks);
	m_activated = true;

	// Request a redraw
	m_dest->need_redraw();
	m_context->started(m_cookie);
	m_context->stopped(m_cookie);
}


bool gui::dx::dx_brush::stop() {
	AM_DBG lib::logger::get_logger()->debug("dx_brush::stop(0x%x)", this);
	if (m_dest) m_dest->renderer_done(this);
	m_dest = NULL;
	m_activated = false;
	m_dxplayer->stopped(this);
	m_context->stopped(m_cookie);
	return true;
}

bool gui::dx::dx_brush::user_event(const lib::point& pt, int what) {
	if (!user_event_sensitive(pt)) return false;
	if(what == common::user_event_click)
		m_context->clicked(m_cookie);
	else if(what == common::user_event_mouse_over) {
		m_context->pointed(m_cookie);
	}
	return true;
}

void gui::dx::dx_brush::redraw(const lib::rect &dirty, common::gui_window *window) {
//	AM_DBG lib::logger::get_logger()->debug("dx_brush::redraw(): %s", repr(dirty).c_str());
	// Get the top-level surface
	dx_window *dxwindow = static_cast<dx_window*>(window);
	viewport *v = dxwindow->get_viewport();
	if(!v) return;

	// Draw the pixels of this renderer to the surface specified by m_dest.
	lib::rect rc = dirty;
	lib::point pt = m_dest->get_global_topleft();
	rc.translate(pt);

	dx_transition *tr = get_transition();
	if (tr && tr->is_fullscreen()) {
		v->set_fullscreen_transition(tr);
		tr = NULL;
	}
	v->clear(rc, m_color, 1.0, tr);
}


