// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2012 Stichting CWI, 
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

#include "ambulant/gui/cocoa/cocoa_ink.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/params.h"
#include "ambulant/lib/tree_builder.h"
#include "ambulant/common/renderer_select.h"
#include "ambulant/smil2/test_attrs.h"

#include <Cocoa/Cocoa.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {


extern const char cocoa_ink_playable_tag[] = "img";
extern const char cocoa_ink_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererInk");
extern const char cocoa_ink_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererCocoa");

common::playable_factory *
create_cocoa_ink_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererInk"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererCocoa"), true);
	return new common::single_playable_factory<
		cocoa_ink_renderer,
		cocoa_ink_playable_tag,
		cocoa_ink_playable_renderer_uri,
		cocoa_ink_playable_renderer_uri2,
		cocoa_ink_playable_renderer_uri2>(factory, mdp);
}

cocoa_ink_renderer::cocoa_ink_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp)
:	cocoa_renderer<renderer_playable_dsall>(context, cookie, node, evp, factory, mdp),
	m_tree(NULL),
	m_path(NULL),
	m_color(NULL),
	m_linewidth(1)
{
	smil2::params *params = smil2::params::for_node(node);
	color_t color = lib::to_color(0, 0, 0);
	if (params) {
		color = params->get_color("color", color);
		float fontsize = (float)params->get_float("font-size", 1.0F); // XXXJACK Abuse fontsize, for now.
		if (fontsize) {
			m_linewidth = fontsize;
		}
		delete params;
	}
	m_color = [NSColor colorWithCalibratedRed:redf(color)
					green:greenf(color)
					blue:bluef(color)
					alpha:1.0f];
}

cocoa_ink_renderer::~cocoa_ink_renderer()
{
	m_lock.enter();
	delete m_tree;
	m_tree = NULL;
	if (m_path) {
		[m_path release];
		m_path = NULL;
	}
	m_lock.leave();
}

void
cocoa_ink_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("cocoa_ink_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	if (m_tree == NULL) {
		// Build the tree first
		const char *str_begin = (const char *)m_data;
		const char *str_end = str_begin + m_data_size;
		lib::tree_builder builder(lib::get_builtin_node_factory());
		if(!builder.build_tree_from_str(str_begin, str_end)) {
			lib::logger::get_logger()->error(gettext("cocoa_ink_renderer: Could not create DOM tree"));
			m_context->stopped(m_cookie);
			return;
		}
		m_tree = builder.detach();
	}
	if (m_path == NULL) {
		m_path = [[NSBezierPath bezierPath] retain];
		[m_path setLineWidth: m_linewidth];
		// XXXJACK These look good for our use cases
		[m_path setLineCapStyle: NSRoundLineCapStyle];
		[m_path setLineJoinStyle: NSRoundLineJoinStyle];
		bool start_new_trace = true;

		// Check root of the tree
		if (m_tree->get_local_name() != "ink") {
			lib::logger::get_logger()->error(gettext("cocoa_ink_renderer: no <ink> found"));
			m_context->stopped(m_cookie);
			return;
		}
		lib::node::const_iterator it;
		lib::node::const_iterator end = m_tree->end();
		for(it = m_tree->begin(); it != end; it++) {
			if(!(*it).first) continue;
			const lib::node *n = (*it).second;
			const std::string& tag = n->get_local_name();
			if (tag == "ink") {
				/* Ignore ink tag */
			} else
			if (tag == "trace") {
				AM_DBG lib::logger::get_logger()->debug("cocoa_ink_renderer: trace");
				start_new_trace = true;
			} else
			if (tag == "point") {
				const char *x_str = n->get_attribute("x");
				const char *y_str = n->get_attribute("y");
				double x = strtod(x_str, NULL);
				double y = strtod(y_str, NULL);
				AM_DBG lib::logger::get_logger()->debug("cocoa_ink_renderer: point (%f,%f)", x, y);
				NSPoint pt = NSMakePoint((float)x, (float)y);
				if (start_new_trace) {
					[m_path moveToPoint: pt];
					start_new_trace = false;
				} else {
					[m_path lineToPoint: pt];
				}
			} else {
				lib::logger::get_logger()->debug("cocoa_ink_renderer: ignoring %s", tag.c_str());
			}
		}
	}
	if (m_color) [m_color set];
	[m_path stroke];
	m_lock.leave();
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant
