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

active_renderer *
gui::none::none_renderer_factory::new_renderer(event_processor *const evp,
	net::passive_datasource *src,
	passive_region *const dest,
	const node *node)
{
	return new ambulant::lib::active_renderer(evp, src, dest, node);
}

passive_window *
gui::none::none_window_factory::new_window(char *name, size bounds)
{
	return new ambulant::lib::passive_window(name, bounds);
}

