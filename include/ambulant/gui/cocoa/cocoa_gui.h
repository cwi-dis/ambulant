
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_GUI_COCOA_COCOA_GUI_H
#define AMBULANT_GUI_COCOA_COCOA_GUI_H

#include "ambulant/lib/region.h"
#include "ambulant/lib/renderer.h"
#ifdef __OBJC__
#include <Cocoa/Cocoa.h>
#endif

namespace ambulant {

namespace gui {

namespace cocoa {

class cocoa_passive_window : public lib::passive_window {
  public:
  	cocoa_passive_window(const std::string &name, lib::size bounds, void *_view)
  	:	lib::passive_window(name, bounds),
  		m_view(_view) {}
  		
	void need_redraw(const lib::screen_rect<int> &r);
	void *view() { return m_view; }
  private:
    void *m_view;
};

class cocoa_window_factory : public lib::window_factory {
  public:
  	cocoa_window_factory(void *view)
  	:	m_view(view) {}
  	
	lib::passive_window *new_window(const std::string &name, lib::size bounds);
  private:
    void *m_view;
};

class cocoa_renderer_factory : public lib::renderer_factory {
  public:
  	cocoa_renderer_factory() {}
  	
	lib::active_renderer *new_renderer(
		lib::event_processor *const evp,
		net::passive_datasource *src,
		lib::passive_region *const dest,
		const lib::node *node);
};

} // namespace cocoa

} // namespace gui
 
} // namespace ambulant

#ifdef __OBJC__
@interface AmbulantView : NSView
{
    ambulant::lib::passive_window *ambulant_window;
}

- (void)setAmbulantWindow: (ambulant::lib::passive_window *)window;
- (NSRect) NSRectForAmbulantRect: (const ambulant::lib::screen_rect<int> *)arect;
- (ambulant::lib::screen_rect<int>) ambulantRectForNSRect: (const NSRect *)nsrect;
@end

#endif // __OBJC__
#endif // AMBULANT_GUI_COCOA_COCOA_GUI_H
