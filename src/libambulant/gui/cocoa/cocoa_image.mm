/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/gui/cocoa/cocoa_image.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

cocoa_active_image_renderer::~cocoa_active_image_renderer()
{
	m_lock.enter();
	AM_DBG logger::get_logger()->trace("~cocoa_active_image_renderer(0x%x)", (void *)this);
	if (m_image)
		[m_image release];
	m_image = NULL;
	m_lock.leave();
}
	
void
cocoa_active_image_renderer::redraw(const screen_rect<int> &dirty, passive_window *window, const point &window_topleft)
{
	m_lock.enter();
	const screen_rect<int> &r = m_dest->get_rect();
	AM_DBG logger::get_logger()->trace("cocoa_active_image_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d), window_topleft=(%d, %d))", (void *)this, r.left(), r.top(), r.right(), r.bottom(), window_topleft.x, window_topleft.y);
	
	if (m_data && !m_image) {
		AM_DBG logger::get_logger()->trace("cocoa_active_image_renderer.redraw: creating image");
		m_nsdata = [NSData dataWithBytesNoCopy: m_data length: m_data_size freeWhenDone: NO];
		m_image = [[NSImage alloc] initWithData: m_nsdata];
		if (!m_image)
			logger::get_logger()->error("cocoa_active_image_renderer.redraw: could not create image");
		// XXXX Could free data and m_data again here...
	}

	cocoa_passive_window *cwindow = (cocoa_passive_window *)window;
	AmbulantView *view = (AmbulantView *)cwindow->view();
	screen_rect<int> window_rect = r;
	window_rect.translate(window_topleft);
	NSRect dstrect = [view NSRectForAmbulantRect: &window_rect];

	if (m_image) {
		NSSize srcsize = [m_image size];
		NSRect srcrect = NSMakeRect(0, 0, srcsize.width, srcsize.height);
		AM_DBG logger::get_logger()->trace("cocoa_active_image_renderer.redraw: draw image %f %f -> (%f, %f, %f, %f)", srcsize.width, srcsize.height, NSMinX(dstrect), NSMinY(dstrect), NSMaxX(dstrect), NSMaxY(dstrect));
		[m_image drawInRect: dstrect fromRect: srcrect operation: NSCompositeCopy fraction: 1.0];
	} else {
		[[NSColor blueColor] set];
		NSRectFill(dstrect);
	}
	m_lock.leave();
}


} // namespace cocoa

} // namespace gui

} //namespace ambulant

