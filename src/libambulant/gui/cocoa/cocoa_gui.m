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

class cocoa_active_text_renderer : active_renderer {
  public:
	cocoa_active_text_renderer(event_processor *const evp,
		net::passive_datasource *src,
		passive_region *const dest,
		const node *node)
	: active_renderer(evp, src, dest, node) {};
	
    void redraw(const screen_rect<int> &r);
};

class cocoa_active_image_renderer : active_renderer {
  public:
	cocoa_active_image_renderer(event_processor *const evp,
		net::passive_datasource *src,
		passive_region *const dest,
		const node *node)
	: active_renderer(evp, src, dest, node) {};

    void redraw(const screen_rect<int> &r);
};

void
cocoa_passive_window::need_redraw(const screen_rect<int> &r)
{
	logger::get_logger()->trace("cocoa_passive_window::need_redraw(0x%x)", (void *)this);
	if (!m_os_window) {
		logger::get_logger()->trace("cocoa_passive_window::need_redraw: no os_window");
		return;
	}
	NSView *my_view = (NSView *)m_os_window;
	[my_view setNeedsDisplay: YES];
}

void
cocoa_active_text_renderer::redraw(const screen_rect<int> &r)
{
	logger::get_logger()->trace("cocoa_active_text_renderer.redraw(0x%x)", (void *)this);
}

void
cocoa_active_image_renderer::redraw(const screen_rect<int> &r)
{
	logger::get_logger()->trace("cocoa_active_image_renderer.redraw(0x%x)", (void *)this);
}

active_renderer *
cocoa_renderer_factory::new_renderer(event_processor *const evp,
	net::passive_datasource *src,
	passive_region *const dest,
	const node *node)
{
	xml_string tag = node->get_qname().second;
	if (tag == "img") {
		return (active_renderer *)new cocoa_active_text_renderer(evp, src, dest, node);
	} else if ( tag == "text") {
		return (active_renderer *)new cocoa_active_image_renderer(evp, src, dest, node);
	}
	return new active_renderer(evp, src, dest, node);
}

passive_window *
cocoa_window_factory::new_window(const std::string &name, size bounds)
{
	return (passive_window *)new cocoa_passive_window(name, bounds, NULL);
}


} // namespace cocoa

} // namespace gui

} //namespace ambulant


