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

#include "ambulant/gui/cocoa/cocoa_text.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/common/region_info.h"
#include "ambulant/smil2/params.h"
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

extern const char cocoa_text_playable_tag[] = "text";
extern const char cocoa_text_playable_renderer_uri[] = AM_SYSTEM_COMPONENT("RendererCocoa");
extern const char cocoa_text_playable_renderer_uri2[] = AM_SYSTEM_COMPONENT("RendererText");

common::playable_factory *
create_cocoa_text_playable_factory(common::factories *factory, common::playable_factory_machdep *mdp)
{
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererCocoa"), true);
	smil2::test_attrs::set_current_system_component_value(AM_SYSTEM_COMPONENT("RendererText"), true);
	return new common::single_playable_factory<
		cocoa_text_renderer,
		cocoa_text_playable_tag,
		cocoa_text_playable_renderer_uri,
		cocoa_text_playable_renderer_uri2,
		cocoa_text_playable_renderer_uri2>(factory, mdp);
}

cocoa_text_renderer::cocoa_text_renderer(
		playable_notification *context,
		playable_notification::cookie_type cookie,
		const lib::node *node,
		event_processor *evp,
		common::factories *factory,
		common::playable_factory_machdep *mdp)
:	cocoa_renderer<renderer_playable_dsall>(context, cookie, node, evp, factory, mdp),
	m_text_storage(NULL),
	m_text_color(0),
	m_font_name(NULL),
	m_font_size(0),
	m_text_font(NULL)
{
	// XXX These parameter names are tentative
	smil2::params *params = smil2::params::for_node(node);
	color_t text_color = lib::to_color(0, 0, 0);
	if (params) {
		const char *fontname = params->get_str("font-family");
//		const char *fontstyle = params->get_str("font-style");
		float fontsize = 0.0F;
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		m_text_color = params->get_color("color", text_color);
		m_font_size = (float)params->get_float("font-size", 0.0F);
		AM_DBG NSLog(@"params found, color=(%d, %d, %d), font-family=%s, font-size=%g",
			redc(text_color), greenc(text_color), bluec(text_color), fontname, fontsize);
		if (fontname)
			m_font_name = [NSString stringWithUTF8String: fontname];
		delete params;
		[pool release];
	}
}

cocoa_text_renderer::~cocoa_text_renderer()
{
	m_lock.enter();
	[m_text_storage release];
	if (m_font_name) [m_font_name release];
	// ??? if (m_text_font) [m_text_font release];
	m_text_storage = NULL;
	m_lock.leave();
}

void
cocoa_text_renderer::redraw_body(const rect &dirty, gui_window *window)
{
	m_lock.enter();
	const rect &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("cocoa_text_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	// Initialize font, if needed
	if ( (m_font_name || m_font_size) && m_text_font == NULL) {
		if (m_font_name) {
			m_text_font = [NSFont fontWithName: m_font_name size: m_font_size];
			if (m_text_font == NULL)
				lib::logger::get_logger()->trace("param: font-family \"%s\" unknown", m_font_name);
		} else	{
			m_text_font = [NSFont userFontOfSize: m_font_size];
			if (m_text_font == NULL)
				lib::logger::get_logger()->trace("param: font-size \"%g\" unknown", m_font_size);
		}
	}
	// Initialize text storage, if needed
	if (m_data && !m_text_storage) {
		unsigned char *ucp = (unsigned char *)m_data;
		// Check for 16-bit unicode by BOM
		NSString *the_string;
		if (m_data_size >= 2 &&
			((ucp[0] == 0xff && ucp[1] == 0xfe) ||
			(ucp[0] == 0xfe && ucp[1] == 0xff)))
		{
			assert((m_data_size & 1) == 0);
			the_string = [NSString stringWithCharacters: (unichar*)m_data length: (unsigned int)(m_data_size/2)];
			assert(the_string);
		} else {
			// Assume it's a utf-8 file.
			char *cdata = (char*)m_data;
			if (cdata[m_data_size] != '\0') {
				cdata = (char *)malloc(m_data_size+1);
				assert(cdata);
				memcpy(cdata, m_data, m_data_size);
				cdata[m_data_size] = '\0';
				the_string = [NSString stringWithUTF8String: cdata];
				assert(the_string);
				free(cdata);
			} else {
				the_string = [NSString stringWithUTF8String: (char *)m_data];
				assert(the_string);
			}
		}
		assert(the_string);
		m_text_storage = [[NSTextStorage alloc] initWithString:the_string];
		if (m_text_color) {
			// XXXJACK will not work, too early for m_dest to be set
			double alfa = 1.0;
			const common::region_info *ri = m_dest->get_info();
			if (ri) alfa = ri->get_mediaopacity();
			NSColor *nscolor = [NSColor colorWithCalibratedRed:redf(m_text_color)
					green:greenf(m_text_color)
					blue:bluef(m_text_color)
					alpha:(float)alfa];
			[m_text_storage setForegroundColor: nscolor];
		}
		if (m_text_font)
			[m_text_storage setFont: m_text_font];
		m_layout_manager = [[NSLayoutManager alloc] init];
		m_text_container = [[NSTextContainer alloc] init];
		[m_layout_manager addTextContainer:m_text_container];
		[m_text_container release]; // The layoutManager will retain the textContainer
		[m_text_storage addLayoutManager:m_layout_manager];
		[m_layout_manager release]; // The textStorage will retain the layoutManager
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
