/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/region.h"
#include "ambulant/lib/renderer.h"
#include "ambulant/lib/logger.h"

using namespace ambulant;

lib::active_region *
lib::passive_region::activate(event_processor *const evp, const node *node)
{
	return new lib::active_region(evp, this, node);
}

void
lib::active_region::show(active_renderer *renderer)
{
	m_renderer = renderer;
	lib::logger::get_logger()->trace("active_region.show(0x%x, \"%s\")", (void *)this, m_source->m_name);
	need_redraw();
}

void
lib::active_region::redraw(const screen_rect<int> &r)
{
	if (m_renderer)
		m_renderer->redraw(r);
	else {
		lib::logger::get_logger()->trace("active_region.redraw(0x%x)", (void *)this);
	}
}

void
lib::active_region::need_redraw(const screen_rect<int> &r)
{
	lib::logger::get_logger()->trace("active_region.need_redraw(0x%x)", (void *)this);
	// XXXX This should be sent to the window interface. For now we short-circuit
	// it ourselves.
	redraw(r);
}

void
lib::active_region::need_redraw()
{
	need_redraw(m_source->m_bounds);
}

void
lib::active_region::done()
{
	m_renderer = NULL;
	lib::logger::get_logger()->trace("active_region.done(0x%x, \"%s\")", (void *)this, m_source->m_name);
	need_redraw();
}
