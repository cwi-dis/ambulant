
/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_COCOA_COCOA_IMAGE_H
#define AMBULANT_GUI_COCOA_COCOA_IMAGE_H

#include "ambulant/common/renderer.h"
#include "ambulant/lib/mtsync.h"
#include <Cocoa/Cocoa.h>

namespace ambulant {

using namespace lib;

namespace gui {

namespace cocoa {

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
	critical_section m_lock;
};

} // namespace cocoa

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_COCOA_COCOA_IMAGE_H
