/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/renderer.h"

namespace ambulant {
namespace lib {
active_renderer * renderer_factory(event_processor *const evp,
	net::passive_datasource *src,
	passive_region *const dest,
	const node *node)
{
	return new ambulant::lib::active_renderer(evp, src, dest, node);
}

} //namespace lib

} //namespace ambulant


