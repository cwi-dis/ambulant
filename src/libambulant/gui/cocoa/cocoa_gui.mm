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

class cocoa_active_text_renderer : public active_final_renderer {
  public:
	cocoa_active_text_renderer(event_processor *const evp,
		net::passive_datasource *src,
		passive_region *const dest,
		const node *node)
	:   active_final_renderer(evp, src, dest, node),
            m_text_storage(NULL) {};
        ~cocoa_active_text_renderer();
	
    void redraw(const screen_rect<int> &dirty, passive_window *window, const point &window_topleft);
  private:
    NSTextStorage *m_text_storage;
	NSLayoutManager *m_layout_manager;
	NSTextContainer *m_text_container;
};

class cocoa_active_image_renderer : public active_final_renderer {
  public:
	cocoa_active_image_renderer(event_processor *const evp,
		net::passive_datasource *src,
		passive_region *const dest,
		const node *node)
	:	active_final_renderer(evp, src, dest, node),
		m_image(NULL),
		m_nsdata(NULL) {};
	~cocoa_active_image_renderer();

    void redraw(const screen_rect<int> &dirty, passive_window *window, const point &window_topleft);
  private:
  	NSImage *m_image;
  	NSData *m_nsdata;
};

void
cocoa_passive_window::need_redraw(const screen_rect<int> &r)
{
	logger::get_logger()->trace("cocoa_passive_window::need_redraw(0x%x, ltrb=(%d,%d,%d,%d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	if (!m_view) {
		logger::get_logger()->trace("cocoa_passive_window::need_redraw: no m_view");
		return;
	}
	AmbulantView *my_view = (AmbulantView *)m_view;
	NSRect my_rect = [my_view NSRectForAmbulantRect: &r];
	[my_view setNeedsDisplayInRect: my_rect];
	//[my_view setNeedsDisplay: YES];
}

cocoa_active_text_renderer::~cocoa_active_text_renderer()
{
	[m_text_storage release];
}

void
cocoa_active_text_renderer::redraw(const screen_rect<int> &dirty, passive_window *window, const point &window_topleft)
{
	const screen_rect<int> &r = m_dest->get_rect();
	logger::get_logger()->trace("cocoa_active_text_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d), topleft=(%d, %d))", (void *)this, r.left(), r.top(), r.right(), r.bottom(), window_topleft.x, window_topleft.y);

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
		logger::get_logger()->trace("cocoa_active_text_renderer.redraw at Cocoa-point (%f, %f)", origin.x, origin.y);
		NSRange glyph_range = [m_layout_manager glyphRangeForTextContainer: m_text_container];
		[m_layout_manager drawBackgroundForGlyphRange: glyph_range atPoint: origin];
		[m_layout_manager drawGlyphsForGlyphRange: glyph_range atPoint: origin];
	} else {
		[[NSColor grayColor] set];
		NSRectFill(dstrect);
	}
            
}

cocoa_active_image_renderer::~cocoa_active_image_renderer()
{
	logger::get_logger()->trace("~cocoa_active_image_renderer(0x%x)", (void *)this);
	if (m_image)
		[m_image release];
	//if (m_nsdata)
	//	[m_nsdata release];
}
	
void
cocoa_active_image_renderer::redraw(const screen_rect<int> &dirty, passive_window *window, const point &window_topleft)
{
	const screen_rect<int> &r = m_dest->get_rect();
	logger::get_logger()->trace("cocoa_active_image_renderer.redraw(0x%x, local_ltrb=(%d,%d,%d,%d), window_topleft=(%d, %d))", (void *)this, r.left(), r.top(), r.right(), r.bottom(), window_topleft.x, window_topleft.y);
	
	if (m_data && !m_image) {
		logger::get_logger()->trace("cocoa_active_image_renderer.redraw: creating image");
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
		logger::get_logger()->trace("cocoa_active_image_renderer.redraw: draw image %f %f -> (%f, %f, %f, %f)", srcsize.width, srcsize.height, NSMinX(dstrect), NSMinY(dstrect), NSMaxX(dstrect), NSMaxY(dstrect));
		[m_image drawInRect: dstrect fromRect: srcrect operation: NSCompositeCopy fraction: 1.0];
	} else {
		[[NSColor blueColor] set];
		NSRectFill(dstrect);
	}
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
		rv = (active_renderer *)new cocoa_active_image_renderer(evp, src, dest, node);
		logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning cocoa_active_image_renderer 0x%x", (void *)node, (void *)rv);
	} else if ( tag == "text") {
		rv = (active_renderer *)new cocoa_active_text_renderer(evp, src, dest, node);
		logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning cocoa_active_text_renderer 0x%x", (void *)node, (void *)rv);
	} else {
		rv = new active_renderer(evp, src, dest, node);
		logger::get_logger()->trace("cocoa_renderer_factory: node 0x%x: returning active_renderer 0x%x", (void *)node, (void *)rv);
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
    NSLog(@"AmbulantView.initWithFrame: self=0x%x", self);
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
    NSLog(@"AmbulantView.drawRect: self=0x%x", self);
    if (!ambulant_window) {
        NSLog(@"Redraw AmbulantView: NULL ambulant_window");
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

