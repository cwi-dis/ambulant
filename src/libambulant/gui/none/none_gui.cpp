/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/gui/none/none_gui.h"
#include "ambulant/lib/renderer.h"
#include "ambulant/lib/region.h"
#include "ambulant/lib/logger.h"

using namespace ambulant;
using namespace lib;

void
gui::none::none_active_renderer::start(lib::event *playdone)
{
	lib::logger::ostream os = lib::logger::get_logger()->trace_stream();
	os << "none_active_renderer.start(" << (void *)this;
	os << ", node=" << *m_node;
	os << ")" << lib::endl;
	lib::active_renderer::start(playdone);
}

void
gui::none::none_active_renderer::redraw(const screen_rect<int> &r, passive_window *window, const point &window_topleft)
{
	lib::logger::get_logger()->trace("none_active_renderer.redraw(0x%x)", (void *)this);
}

void
gui::none::none_active_renderer::stop()
{
	lib::logger::get_logger()->trace("none_active_renderer.stop(0x%x)", (void *)this);
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

