/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/gui/cocoa/cocoa_gui.h"
#include "ambulant/lib/renderer.h"

#include <Cocoa/Cocoa.h>

namespace ambulant {

using namespace lib;

namespace gui {
namespace cocoa {

class cocoa_active_text_renderer : active_final_renderer {
  public:
	cocoa_active_text_renderer(event_processor *const evp,
		net::passive_datasource *src,
		passive_region *const dest,
		const node *node)
	:	active_final_renderer(evp, src, dest, node) {};
	
    void redraw(const screen_rect<int> &r);
};

class cocoa_active_image_renderer : active_final_renderer {
  public:
	cocoa_active_image_renderer(event_processor *const evp,
		net::passive_datasource *src,
		passive_region *const dest,
		const node *node)
	:	active_final_renderer(evp, src, dest, node),
		m_image(NULL) {};
	~cocoa_active_image_renderer();

    void redraw(const screen_rect<int> &r);
  private:
  	NSImage *m_image;
};

void
cocoa_passive_window::need_redraw(const screen_rect<int> &r)
{
	logger::get_logger()->trace("cocoa_passive_window::need_redraw(0x%x, ltrb=(%d,%d,%d,%d))", (void *)this, r.left, r.top, r.right, r.bottom);
	if (!m_view) {
		logger::get_logger()->trace("cocoa_passive_window::need_redraw: no m_view");
		return;
	}
	NSView *my_view = (NSView *)m_view;
	// XXXX Should use setNeedsDisplayInRect:
	[my_view setNeedsDisplay: YES];
}

void
cocoa_active_text_renderer::redraw(const screen_rect<int> &r)
{
	logger::get_logger()->trace("cocoa_active_text_renderer.redraw(0x%x, ltrb=(%d,%d,%d,%d))", (void *)this, r.left, r.top, r.right, r.bottom);
	[[NSColor redColor] set];
	NSRectFill(NSMakeRect(r.left, r.top, r.right-r.left, r.bottom-r.top));
}

cocoa_active_image_renderer::~cocoa_active_image_renderer()
{
	if (m_image)
		[m_image dealloc];
}
	
void
cocoa_active_image_renderer::redraw(const screen_rect<int> &r)
{
	logger::get_logger()->trace("cocoa_active_image_renderer.redraw(0x%x, ltrb=(%d,%d,%d,%d))", (void *)this, r.left, r.top, r.right, r.bottom);
	if (m_data && !m_image) {
		logger::get_logger()->trace("cocoa_active_image_renderer.redraw: creating image");
		NSData *data = [NSData dataWithBytesNoCopy: m_data length: m_data_size];
		m_image = [[NSImage alloc] initWithData: data];
		// XXXX Could free data and m_data again here...
	}
	[[NSColor blueColor] set];
	NSRectFill(NSMakeRect(r.left, r.top, r.right-r.left, r.bottom-r.top));
	if (m_image) {
		logger::get_logger()->trace("cocoa_active_image_renderer.redraw: draw image");
	}
}

active_renderer *
cocoa_renderer_factory::new_renderer(event_processor *const evp,
	net::passive_datasource *src,
	passive_region *const dest,
	const node *node)
{
	xml_string tag = node->get_qname().second;
	if (tag == "img") {
		logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning cocoa_active_image_renderer", (void *)node);
		return (active_renderer *)new cocoa_active_text_renderer(evp, src, dest, node);
	} else if ( tag == "text") {
		logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning cocoa_active_text_renderer", (void *)node);
		return (active_renderer *)new cocoa_active_image_renderer(evp, src, dest, node);
	}
	logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning active_renderer", (void *)node);
	return new active_renderer(evp, src, dest, node);
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
    NSLog(@"AmbulantView.initWithFrame: self=0x%x", self);
    return self;
}

- (void)drawRect:(NSRect)rect
{
    NSLog(@"AmbulantView.drawRect: self=0x%x", self);
    if (!ambulant_window) {
        NSLog(@"Redraw AmbulantView: NULL ambulant_window");
        [[NSColor grayColor] set];
        NSRectFill([self bounds]);
    } else {
        ambulant::lib::screen_rect<int> arect = ambulant::lib::screen_rect<int>(
                int(NSMinX(rect)), int(NSMinY(rect)),
                int(NSMaxX(rect)), int(NSMaxY(rect)));
        ambulant_window->redraw(arect);
    }
}

- (void)setAmbulantWindow: (ambulant::lib::passive_window *)window
{
    ambulant_window = window;
}

@end
#endif // __OBJC__

