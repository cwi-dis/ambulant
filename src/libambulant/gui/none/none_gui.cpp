/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/gui/none/none_gui.h"
#include "ambulant/lib/renderer.h"
#include "ambulant/lib/region.h"

using namespace ambulant;
using namespace lib;

class none_active_renderer : public active_renderer {
  public:
	none_active_renderer(event_processor *const evp,
		net::passive_datasource *src,
		passive_region *const dest,
		const node *node)
	:	active_renderer(evp, src, dest, node) {};
	
	void redraw(const screen_rect<int> &r, passive_window *window, const point &window_topleft);
};

void
none_active_renderer::redraw(const screen_rect<int> &r, passive_window *window, const point &window_topleft)
{
	lib::logger::get_logger()->trace("none_active_renderer.redraw(0x%x)", (void *)this);
}

active_renderer *
gui::none::none_renderer_factory::new_renderer(event_processor *const evp,
	net::passive_datasource *src,
	passive_region *const dest,
	const node *node)
{
	return new none_active_renderer(evp, src, dest, node);
}

passive_window *
gui::none::none_window_factory::new_window(const std::string &name, size bounds)
{
	return new ambulant::lib::passive_window(name, bounds);
}

