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
#include "ambulant/gui/cocoa/cocoa_fill.h"
#include "ambulant/gui/cocoa/cocoa_mouse.h"
#include "ambulant/lib/mtsync.h"

#include <Cocoa/Cocoa.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

cocoa_window::~cocoa_window()
{
	if (m_view) {
		AmbulantView *my_view = (AmbulantView *)m_view;
		[my_view ambulantWindowClosed];
	}
}
	
void
cocoa_window::need_redraw(const screen_rect<int> &r)
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

#if 0
void
cocoa_window::need_events(lib::abstract_mouse_region *rgn)
{
	AM_DBG logger::get_logger()->trace("cocoa_passive_window::need_events(0x%x)", (void *)this);
	if (!m_view) {
		logger::get_logger()->fatal("cocoa_passive_window::need_redraw: no m_view");
		return;
	}
	AmbulantView *my_view = (AmbulantView *)m_view;
//	NSRect my_rect = [my_view NSRectForAmbulantRect: &r];
//	[my_view setNeedsDisplayInRect: my_rect];
//	//[my_view setNeedsDisplay: YES];
}
#endif

void
cocoa_window::redraw(const screen_rect<int> &r)
{
	AM_DBG logger::get_logger()->trace("cocoa_window::redraw(0x%x, ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	m_region->redraw(r, this);
}

void
cocoa_window::user_event(const point &where)
{
	AM_DBG logger::get_logger()->trace("cocoa_window::user_event(0x%x, (%d, %d))", (void *)this, where.x, where.y);
	m_region->user_event(where);
}

void
cocoa_window::mouse_region_changed()
{
	AM_DBG logger::get_logger()->trace("cocoa_window::mouse_region_changed(0x%x)", (void *)this);
	AmbulantView *my_view = (AmbulantView *)m_view;
        NSWindow *my_window = [my_view window];
	[my_window invalidateCursorRectsForView: my_view];
}

active_basic_renderer *
cocoa_renderer_factory::new_renderer(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	event_processor *const evp,
	net::passive_datasource *src,
	abstract_rendering_surface *const dest)
{
	active_basic_renderer *rv;
	
	xml_string tag = node->get_qname().second;
	if (tag == "img") {
		rv = new cocoa_active_image_renderer(context, cookie, node, evp, src, dest);
		AM_DBG logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning cocoa_active_image_renderer 0x%x", (void *)node, (void *)rv);
	} else if ( tag == "text") {
		rv = new cocoa_active_text_renderer(context, cookie, node, evp, src, dest);
		AM_DBG logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning cocoa_active_text_renderer 0x%x", (void *)node, (void *)rv);
	} else if ( tag == "brush") {
		rv = new cocoa_active_fill_renderer(context, cookie, node, evp, src, dest);
		AM_DBG logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning cocoa_active_fill_renderer 0x%x", (void *)node, (void *)rv);
#ifdef WITH_COCOA_AUDIO
	} else if ( tag == "audio") {
		rv = new cocoa_active_audio_renderer(context, cookie, node, evp, src);
		AM_DBG logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning cocoa_active_audio_renderer 0x%x", (void *)node, (void *)rv);
#endif
	} else {
		// logger::get_logger()->error("cocoa_renderer_factory: no Cocoa renderer for tag \"%s\"", tag.c_str());
                return NULL;
	}
	return rv;
}

abstract_window *
cocoa_window_factory::new_window(const std::string &name, size bounds, abstract_rendering_source *region)
{
	if ([(AmbulantView *)m_defaultwindow_view isAmbulantWindowInUse]) {
		// XXXX Should create new toplevel window and put an ambulantview in it
		logger::get_logger()->error("cocoa_window_factory: cannot open second toplevel window yet");
		return NULL;
	}
	cocoa_window *window = new cocoa_window(name, bounds, m_defaultwindow_view, region);
	// And we need to inform the object about it
	AmbulantView *view = window->view();
	// And set the window size
	[view setAmbulantWindow: window];
	NSSize cocoa_size = NSMakeSize(bounds.w, bounds.h);
	[[view window] setContentSize: cocoa_size];
	AM_DBG NSLog(@"Size changed on %@ to (%f, %f)", [view window], cocoa_size.width, cocoa_size.height);
	AM_DBG NSLog(@"Calling mouse_region_changed");
	window->mouse_region_changed();
	[[view window] makeKeyAndOrderFront: view];
	return (abstract_window *)window;
}

abstract_mouse_region *
cocoa_window_factory::new_mouse_region()
{
	return new cocoa_mouse_region();
};

abstract_bg_rendering_source *
cocoa_window_factory::new_background_renderer()
{
	return new cocoa_background_renderer();
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
#ifdef USE_COCOA_BOTLEFT
	float bot_delta = NSMaxY([self bounds]) - arect->bottom();
	return NSMakeRect(arect->left(), bot_delta, arect->width(), arect->height());
#else
	return NSMakeRect(arect->left(), arect->top(), arect->width(), arect->height());
#endif
}

- (ambulant::lib::screen_rect<int>) ambulantRectForNSRect: (const NSRect *)nsrect
{
#ifdef USE_COCOA_BOTLEFT
	float top_delta = NSMaxY([self bounds]) - NSMaxY(*nsrect);
	ambulant::lib::screen_rect<int> arect = ambulant::lib::screen_rect<int>(
                ambulant::lib::point(int(NSMinX(*nsrect)), int(top_delta)),
				ambulant::lib::size(int(NSWidth(*nsrect)), int(NSHeight(*nsrect))));
#else
	ambulant::lib::screen_rect<int> arect = ambulant::lib::screen_rect<int>(
                ambulant::lib::point(int(NSMinX(*nsrect)), int(NSMinY(*nsrect))),
				ambulant::lib::size(int(NSWidth(*nsrect)), int(NSHeight(*nsrect))));
	 
#endif
	return arect;
}

- (void)drawRect:(NSRect)rect
{
    AM_DBG NSLog(@"AmbulantView.drawRect: self=0x%x", self);
    if (!ambulant_window) {
        AM_DBG NSLog(@"Redraw AmbulantView: NULL ambulant_window");
    } else {
        ambulant::lib::screen_rect<int> arect = [self ambulantRectForNSRect: &rect];
        ambulant_window->redraw(arect);
    }
}

- (void)setAmbulantWindow: (ambulant::gui::cocoa::cocoa_window *)window
{
    ambulant_window = window;
}

- (void)ambulantWindowClosed
{
    AM_DBG NSLog(@"ambulantWindowClosed called");
    ambulant_window = NULL;
	// XXXX Should we close the window too? Based on preference?
}

- (bool)isAmbulantWindowInUse
{
    return (ambulant_window != NULL);
}

- (BOOL)isFlipped
{
#ifdef USE_COCOA_BOTLEFT
	return false;
#else
	return true;
#endif
}

- (void)resetCursorRects
{
	bool want_events = false;
	if ( ambulant_window ) {
		const ambulant::common::abstract_mouse_region &mrgn = ambulant_window->get_mouse_region();
		want_events = !mrgn.is_empty();
	}
	AM_DBG NSLog(@"resetCursorRects wantevents=%d", (int)want_events);
	if (want_events) [self addCursorRect: [self visibleRect] cursor: [NSCursor pointingHandCursor]];
}

- (void)mouseDown: (NSEvent *)theEvent
{
	NSPoint where = [theEvent locationInWindow];
	AM_DBG NSLog(@"mouseDown at (%f, %f)", where.x, where.y);
	ambulant::lib::point amwhere = ambulant::lib::point((int)where.x, (int)where.y);
	if (ambulant_window) ambulant_window->user_event(amwhere);
}
@end
#endif // __OBJC__

