
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

namespace ambulant {

namespace gui {

namespace cocoa {

class cocoa_passive_window : public lib::passive_window {
  public:
  	cocoa_passive_window(const std::string &name, lib::size bounds, void *os_window)
  	:	lib::passive_window(name, bounds),
  		m_os_window(os_window) {}
  		
	void need_redraw(const lib::screen_rect<int> &r);
  private:
    void *m_os_window;
};

class cocoa_window_factory : lib::window_factory {
  public:
  	cocoa_window_factory() {}
  	
	lib::passive_window *new_window(const std::string &name, lib::size bounds);
};

class cocoa_renderer_factory : lib::renderer_factory {
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

#endif // AMBULANT_GUI_COCOA_COCOA_GUI_H
