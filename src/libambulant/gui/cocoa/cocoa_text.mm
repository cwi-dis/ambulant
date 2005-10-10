// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
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

#include "ambulant/gui/cocoa/cocoa_text.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/params.h"

#include <Cocoa/Cocoa.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

cocoa_text_renderer::cocoa_text_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory)
:	cocoa_renderer<renderer_playable_dsall>(context, cookie, node, evp, factory),
	m_text_storage(NULL),
	m_text_color(NULL),
	m_text_font(NULL)
{
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
}

cocoa_text_renderer::~cocoa_text_renderer()
{
	m_lock.enter();
	[m_text_storage release];
	m_text_storage = NULL;
	m_lock.leave();
}

void
cocoa_text_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("cocoa_text_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	if (m_data && !m_text_storage) {
		NSString *the_string = [NSString stringWithCString: (char *)m_data length: m_data_size];
		m_text_storage = [[NSTextStorage alloc] initWithString:the_string];
		if (m_text_color)
			[m_text_storage setForegroundColor: m_text_color];
		if (m_text_font)
			[m_text_storage setFont: m_text_font];
		m_layout_manager = [[NSLayoutManager alloc] init];
		m_text_container = [[NSTextContainer alloc] init];
		[m_layout_manager addTextContainer:m_text_container];
		[m_text_container release];	// The layoutManager will retain the textContainer
		[m_text_storage addLayoutManager:m_layout_manager];
		[m_layout_manager release];	// The textStorage will retain the layoutManager
	}

	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	rect dstrect = r;
	dstrect.translate(m_dest->get_global_topleft());
	NSRect cocoa_dstrect = [view NSRectForAmbulantRect: &dstrect];
	if (m_text_storage && m_layout_manager) {
		NSPoint origin = NSMakePoint(NSMinX(cocoa_dstrect), NSMinY(cocoa_dstrect));
		NSSize size = NSMakeSize(NSWidth(cocoa_dstrect), NSHeight(cocoa_dstrect));
		if (1 /*size != [m_text_container containerSize]*/) {
			AM_DBG logger::get_logger()->debug("cocoa_text_renderer.redraw: setting size to (%f, %f)", size.width, size.height);
			[m_text_container setContainerSize: size];
		}
		AM_DBG logger::get_logger()->debug("cocoa_text_renderer.redraw at Cocoa-point (%f, %f)", origin.x, origin.y);
		NSRange glyph_range = [m_layout_manager glyphRangeForTextContainer: m_text_container];
		//[m_layout_manager drawBackgroundForGlyphRange: glyph_range atPoint: origin];
		[m_layout_manager drawGlyphsForGlyphRange: glyph_range atPoint: origin];
	}
	m_lock.leave();
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant
