
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
cocoa_renderer_factory::new_renderer(event_processor *const evp,
	net::passive_datasource *src,
	passive_region *const dest,
	const node *node)
{
	active_renderer *rv;
	
	xml_string tag = node->get_qname().second;
	if (tag == "img") {
		rv = new cocoa_active_image_renderer(evp, src, dest, node);
		AM_DBG logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning cocoa_active_image_renderer 0x%x", (void *)node, (void *)rv);
	} else if ( tag == "text") {
		rv = new cocoa_active_text_renderer(evp, src, dest, node);
		AM_DBG logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning cocoa_active_text_renderer 0x%x", (void *)node, (void *)rv);
	} else if ( tag == "audio") {
		rv = (active_renderer *)new cocoa_active_audio_renderer(evp, src, node);
		AM_DBG logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning cocoa_active_audio_renderer 0x%x", (void *)node, (void *)rv);
	} else {
		logger::get_logger()->error("cocoa_renderer_factory: no Cocoa renderer for tag \"%s\"", tag.c_str());
		rv = new gui::none::none_active_renderer(evp, src, dest, node);
		AM_DBG logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning none_active_renderer 0x%x", (void *)node, (void *)rv);
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
        [[NSColor grayColor] set];
        NSRectFill([self bounds]);
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

