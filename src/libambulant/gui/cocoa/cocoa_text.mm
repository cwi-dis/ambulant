/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/gui/cocoa/cocoa_text.h"
#include "ambulant/gui/cocoa/cocoa_gui.h"

#include <Cocoa/Cocoa.h>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

cocoa_active_text_renderer::~cocoa_active_text_renderer()
{
	m_lock.enter();
	[m_text_storage release];
	m_text_storage = NULL;
	m_lock.leave();
}

void
cocoa_active_text_renderer::redraw(const screen_rect<int> &dirty, passive_window *window, const point &window_topleft)
{
	m_lock.enter();
	const screen_rect<int> &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->trace("cocoa_active_text_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d), topleft=(%d, %d))", (void *)this, r.left(), r.top(), r.right(), r.bottom(), window_topleft.x, window_topleft.y);

	if (m_data && !m_text_storage) {
		NSString *the_string = [NSString stringWithCString: (char *)m_data length: m_data_size];
		m_text_storage = [[NSTextStorage alloc] initWithString:the_string];
		m_layout_manager = [[NSLayoutManager alloc] init];
		m_text_container = [[NSTextContainer alloc] init];
		[m_layout_manager addTextContainer:m_text_container];
		[m_text_container release];	// The layoutManager will retain the textContainer
		[m_text_storage addLayoutManager:m_layout_manager];
		[m_layout_manager release];	// The textStorage will retain the layoutManager
	}

	cocoa_passive_window *cwindow = (cocoa_passive_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	screen_rect<int> window_rect = r;
	window_rect.translate(window_topleft);
	NSRect dstrect = [view NSRectForAmbulantRect: &window_rect];
	
	if (m_text_storage && m_layout_manager) {
		[[NSColor whiteColor] set];
		NSRectFill(dstrect);
		NSPoint origin = NSMakePoint(NSMinX(dstrect), NSMidY(dstrect));
		AM_DBG logger::get_logger()->trace("cocoa_active_text_renderer.redraw at Cocoa-point (%f, %f)", origin.x, origin.y);
		NSRange glyph_range = [m_layout_manager glyphRangeForTextContainer: m_text_container];
		[m_layout_manager drawBackgroundForGlyphRange: glyph_range atPoint: origin];
		[m_layout_manager drawGlyphsForGlyphRange: glyph_range atPoint: origin];
	} else {
		[[NSColor grayColor] set];
		NSRectFill(dstrect);
	}
	m_lock.leave();
}

} // namespace cocoa

} // namespace gui

} //namespace ambulant
