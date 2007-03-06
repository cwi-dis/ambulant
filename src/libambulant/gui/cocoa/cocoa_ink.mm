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

#include "ambulant/gui/cocoa/cocoa_ink.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/params.h"
#include "ambulant/lib/tree_builder.h"

#include <Cocoa/Cocoa.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

cocoa_ink_renderer::cocoa_ink_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory)
:	cocoa_renderer<renderer_playable_dsall>(context, cookie, node, evp, factory),
	m_tree(NULL)
{
#if 0
	// XXX These parameter names are tentative
	smil2::params *params = smil2::params::for_node(node);
	color_t text_color = lib::to_color(0, 0, 0);
	if (params) {
		const char *fontname = params->get_str("font-family");
//		const char *fontstyle = params->get_str("font-style");
		float fontsize = 0.0;
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		text_color = params->get_color("color", text_color);
		fontsize = params->get_float("font-size", 0.0);
		AM_DBG NSLog(@"params found, color=(%d, %d, %d), font-family=%s, font-size=%g", 
			redc(text_color), greenc(text_color), bluec(text_color), fontname, fontsize);
		if (fontname) {
			NSString *nsfontname = [NSString stringWithCString: fontname];
			m_text_font = [NSFont fontWithName: nsfontname size: fontsize];
			if (m_text_font == NULL)
				lib::logger::get_logger()->trace("param: font-family \"%s\" unknown", fontname);
		} else if (fontsize) {
			m_text_font = [NSFont userFontOfSize: fontsize];
			if (m_text_font == NULL)
				lib::logger::get_logger()->trace("param: font-size \"%g\" unknown", fontsize);
		}
		delete params;
		[pool release];
	}
	m_text_color = [NSColor colorWithCalibratedRed:redf(text_color)
					green:greenf(text_color)
					blue:bluef(text_color)
					alpha:1.0];
#endif
}

cocoa_ink_renderer::~cocoa_ink_renderer()
{
	m_lock.enter();
#if 0
	[m_text_storage release];
	m_text_storage = NULL;
#endif
	m_lock.leave();
}

void
cocoa_ink_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	const rect &r = m_dest->get_rect();
	/*AM_DBG*/ logger::get_logger()->debug("cocoa_ink_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
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
		m_tree = builder.get_tree();
	}
#if 0
	// Check root of the tree
	if (m_tree->get_local_name() != "ink") {
		lib::logger::get_logger()->error(gettext("cocoa_ink_renderer: no <ink> found"));
		m_context->stopped(m_cookie);
		return;
	}
#endif
	lib::node::const_iterator it;
	lib::node::const_iterator end = m_tree->end();
	for(it = m_tree->begin(); it != end; it++) {
		if(!(*it).first) continue;
		const lib::node *n = (*it).second;
		const std::string& tag = n->get_local_name();
		if (tag == "trace") {
			/*AM_DBG*/ lib::logger::get_logger()->debug("cocoa_ink_renderer: trace");
		} else
		if (tag == "point") {
			const char *x_str = n->get_attribute("x");
			const char *y_str = n->get_attribute("y");
			/*AM_DBG*/ lib::logger::get_logger()->debug("cocoa_ink_renderer:     point (%s,%s)", x_str, y_str);
		} else {
			/*AM_DBG*/ lib::logger::get_logger()->debug("cocoa_ink_renderer: ignoring %s", tag.c_str());
		}
	}
	m_lock.leave();
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant
