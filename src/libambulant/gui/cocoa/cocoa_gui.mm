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

#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/gui/cocoa/cocoa_audio.h"
#include "ambulant/gui/cocoa/cocoa_text.h"
#include "ambulant/gui/cocoa/cocoa_image.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/common/renderer.h"
#include "ambulant/lib/mtsync.h"

#include <Cocoa/Cocoa.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

void
cocoa_passive_window::need_redraw(const screen_rect<int> &r)
{
	AM_DBG logger::get_logger()->trace("cocoa_passive_window::need_redraw(0x%x, ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	if (!m_view) {
		logger::get_logger()->fatal("cocoa_passive_window::need_redraw: no m_view");
		return;
	}
	AmbulantView *my_view = (AmbulantView *)m_view;
	NSRect my_rect = [my_view NSRectForAmbulantRect: &r];
	[my_view setNeedsDisplayInRect: my_rect];
	//[my_view setNeedsDisplay: YES];
}

active_renderer *
cocoa_renderer_factory::new_renderer(
	active_playable_events *context,
	active_playable_events::cookie_type cookie,
	const node *node,
	event_processor *const evp,
	net::passive_datasource *src,
	passive_region *const dest)
{
	active_renderer *rv;
	
	xml_string tag = node->get_qname().second;
	if (tag == "img") {
		rv = new cocoa_active_image_renderer(context, cookie, node, evp, src, dest);
		AM_DBG logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning cocoa_active_image_renderer 0x%x", (void *)node, (void *)rv);
	} else if ( tag == "text") {
		rv = new cocoa_active_text_renderer(context, cookie, node, evp, src, dest);
		AM_DBG logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning cocoa_active_text_renderer 0x%x", (void *)node, (void *)rv);
#ifdef WITH_COCOA_AUDIO
	} else if ( tag == "audio") {
		rv = (active_renderer *)new cocoa_active_audio_renderer(context, cookie, node, evp, src);
		AM_DBG logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning cocoa_active_audio_renderer 0x%x", (void *)node, (void *)rv);
#endif
	} else {
		// logger::get_logger()->error("cocoa_renderer_factory: no Cocoa renderer for tag \"%s\"", tag.c_str());
                return NULL;
	}
	return rv;
}

passive_window *
cocoa_window_factory::new_window(const std::string &name, size bounds)
{
	passive_window *window = (passive_window *)new cocoa_passive_window(name, bounds, m_view);
	// And we need to inform the object about it
	AmbulantView *view = (AmbulantView *)m_view;
	[view setAmbulantWindow: window];
	return window;
}


} // namespace cocoa

} // namespace gui

} //namespace ambulant

#ifdef __OBJC__
@implementation AmbulantView

- (id)initWithFrame:(NSRect)frameRect
{
    [super initWithFrame:frameRect];
    ambulant_window = NULL;
    AM_DBG NSLog(@"AmbulantView.initWithFrame: self=0x%x", self);
    return self;
}
- (NSRect) NSRectForAmbulantRect: (const ambulant::lib::screen_rect<int> *)arect
{
	float bot_delta = NSMaxY([self bounds]) - arect->bottom();
	return NSMakeRect(arect->left(), bot_delta, arect->width(), arect->height());
}

- (ambulant::lib::screen_rect<int>) ambulantRectForNSRect: (const NSRect *)nsrect
{
	float top_delta = NSMaxY([self bounds]) - NSMaxY(*nsrect);
	ambulant::lib::screen_rect<int> arect = ambulant::lib::screen_rect<int>(
                ambulant::lib::point(int(NSMinX(*nsrect)), int(top_delta)),
				ambulant::lib::size(int(NSWidth(*nsrect)), int(NSHeight(*nsrect))));
	return arect;
}

- (void)drawRect:(NSRect)rect
{
    AM_DBG NSLog(@"AmbulantView.drawRect: self=0x%x", self);
    if (!ambulant_window) {
        AM_DBG NSLog(@"Redraw AmbulantView: NULL ambulant_window");
    } else {
        ambulant::lib::screen_rect<int> arect = [self ambulantRectForNSRect: &rect];
        ambulant_window->redraw(arect, ambulant_window);
    }
}

- (void)setAmbulantWindow: (ambulant::lib::passive_window *)window
{
    ambulant_window = window;
}

@end
#endif // __OBJC__

