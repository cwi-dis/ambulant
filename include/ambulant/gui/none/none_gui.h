
/* 
 * @$Id$ 
 */

#ifndef AMBULANT_GUI_NONE_NONE_GUI_H
#define AMBULANT_GUI_NONE_NONE_GUI_H

#include "ambulant/common/region.h"
#include "ambulant/common/renderer.h"

namespace ambulant {

namespace gui {

namespace none {

class none_window_factory : public lib::window_factory {
  public:
  	none_window_factory() {}
  	
	lib::passive_window *new_window(const std::string &name, lib::size bounds);
};

class none_active_renderer : public lib::active_renderer {
  public:
	none_active_renderer(lib::event_processor *const evp,
		net::passive_datasource *src,
		lib::passive_region *const dest,
		const lib::node *node)
	:	lib::active_renderer(evp, src, dest, node) {};
	
	void start(lib::event *playdone);
	void redraw(const lib::screen_rect<int> &r, lib::passive_window *window, const lib::point &window_topleft);
	void stop();
};

class none_renderer_factory : public lib::renderer_factory {
  public:
  	none_renderer_factory() {}
  	
	lib::active_renderer *new_renderer(
		lib::event_processor *const evp,
		net::passive_datasource *src,
		lib::passive_region *const dest,
		const lib::node *node);
};

} // namespace none

} // namespace gui
 
} // namespace ambulant

#endif // AMBULANT_GUI_NONE_NONE_GUI_H
