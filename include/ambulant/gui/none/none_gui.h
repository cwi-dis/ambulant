
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_GUI_NONE_NONE_GUI_H
#define AMBULANT_GUI_NONE_NONE_GUI_H

#include "ambulant/lib/region.h"
#include "ambulant/lib/renderer.h"

namespace ambulant {

namespace gui {

namespace none {

class none_window_factory : lib::window_factory {
  public:
  	none_window_factory() {}
  	
	lib::passive_window *new_window(char *name, lib::size bounds);
};

class none_renderer_factory : lib::renderer_factory {
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
