
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_GUI_COCOA_COCOA_TEXT_H
#define AMBULANT_GUI_COCOA_COCOA_TEXT_H

#include "ambulant/lib/renderer.h"
#include "ambulant/lib/mtsync.h"
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
	critical_section m_lock;
};

} // namespace cocoa

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_COCOA_COCOA_TEXT_H
