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
:	cocoa_renderer(context, cookie, node, evp, factory),
	m_text_storage(NULL)
{
	smil2::params *params = smil2::params::for_node(node);
	color_t *text_color = lib::to_color(0, 0, 0);
	if (params) {
		text_color = params->get_color("fontColor", text_color);
		NSLog(@"params found, fontColor=(%d, %d, %d)", redc(text_color), greenc(text_color), bluec(text_color));
		delete params;
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
cocoa_text_renderer::redraw_body(const screen_rect<int> &dirty, gui_window *window)
{
	m_lock.enter();
	const screen_rect<int> &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->debug("cocoa_text_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());

	if (m_data && !m_text_storage) {
		NSString *the_string = [NSString stringWithCString: (char *)m_data length: m_data_size];
		m_text_storage = [[NSTextStorage alloc] initWithString:the_string];
		[m_text_storage setForegroundColor: m_text_color];
		m_layout_manager = [[NSLayoutManager alloc] init];
		m_text_container = [[NSTextContainer alloc] init];
		[m_layout_manager addTextContainer:m_text_container];
		[m_text_container release];	// The layoutManager will retain the textContainer
		[m_text_storage addLayoutManager:m_layout_manager];
		[m_layout_manager release];	// The textStorage will retain the layoutManager
	}

	cocoa_window *cwindow = (cocoa_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	screen_rect<int> dstrect = r;
	dstrect.translate(m_dest->get_global_topleft());
	NSRect cocoa_dstrect = [view NSRectForAmbulantRect: &dstrect];
#if 0
	// XXXX WRONG! This is the info for the region, not for the node!
	const region_info *info = m_dest->get_info();
	// First find our whole area
	if (info && !info->get_transparent()) {
		// XXXX Fill with background color
		color_t bgcolor = info->get_bgcolor();
		AM_DBG lib::logger::get_logger()->debug("cocoa_text_renderer.redraw: clearing to 0x%x", (long)bgcolor);
		NSColor *cocoa_bgcolor = [NSColor colorWithCalibratedRed:redf(bgcolor)
					green:greenf(bgcolor)
					blue:bluef(bgcolor)
					alpha:1.0];
		[cocoa_bgcolor set];
		NSRectFill(cocoa_dstrect);
	}
#endif
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
