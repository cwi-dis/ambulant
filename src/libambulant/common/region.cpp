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

lib::passive_region *
lib::passive_region::subregion(const std::string &name, screen_rect<int> bounds)
{
	passive_region *rv = new passive_region(name, this, bounds);
	m_children.push_back(rv);
	return rv;
}

lib::active_region *
lib::passive_region::activate(event_processor *const evp, const node *node)
{
	active_region *rv = new lib::active_region(evp, this, node);
	return rv;
}

void
lib::passive_region::show(active_region *cur)
{
	m_cur_active_region = cur;
	lib::logger::get_logger()->trace("passive_region.show(0x%x, active=0x%x)", (void *)this, (void *)m_cur_active_region);
}

void
lib::passive_region::redraw(const screen_rect<int> &r)
{
	lib::logger::get_logger()->trace("passive_region.redraw(0x%x, ltrb=(%d, %d, %d, %d), bounds=(%d, %d, %d, %d))", (void *)this, r.left, r.top, r.right, r.bottom, m_bounds.left, m_bounds.top, m_bounds.right, m_bounds.bottom);
	screen_rect<int> our_rect = r & m_bounds;
	if (our_rect.empty())
		return;
	lib::logger::get_logger()->trace("passive_region.redraw(0x%x, our_ltrb=(%d, %d, %d, %d))", (void *)this, our_rect.left, our_rect.top, our_rect.right, our_rect.bottom);
		
	if (m_cur_active_region) {
		lib::logger::get_logger()->trace("passive_region.redraw(0x%x) ->active 0x%x", (void *)this, (void *)m_cur_active_region);
		m_cur_active_region->redraw(our_rect);
	} else {
		lib::logger::get_logger()->trace("passive_region.redraw(0x%x) no active region", (void *)this);
	}
	std::vector<passive_region *>::iterator i;
	for(i=m_children.begin(); i<m_children.end(); i++) {
		lib::logger::get_logger()->trace("passive_region.redraw(0x%x) -> child 0x%x", (void *)this, (void *)(*i));
		(*i)->redraw(our_rect);
	}
}

void
lib::passive_region::need_redraw(const screen_rect<int> &r)
{
	if (!m_parent)
		return;   // Audio region or some such
	m_parent->need_redraw(r);
}

void
lib::passive_window::need_redraw(const screen_rect<int> &r)
{
	lib::logger::get_logger()->trace("passive_window.need_redraw(0x%x)", (void *)this);
	// XXXX This should be sent to the window interface. For now we short-circuit
	// it ourselves.
	redraw(r);
}

void
lib::active_region::show(active_renderer *renderer)
{
	m_renderer = renderer;
	m_source->show(this);
	lib::logger::get_logger()->trace("active_region.show(0x%x, \"%s\", renderer=0x%x)", (void *)this, m_source->m_name.c_str(), (void *)renderer);
	need_redraw();
}

void
lib::active_region::redraw(const screen_rect<int> &r)
{
	if (m_renderer) {
		lib::logger::get_logger()->trace("active_region.redraw(0x%x) -> renderer 0x%x", (void *)this, (void *)m_renderer);
		m_renderer->redraw(r);
	} else {
		lib::logger::get_logger()->trace("active_region.redraw(0x%x) no renderer", (void *)this);
	}
}

void
lib::active_region::need_redraw(const screen_rect<int> &r)
{
	// XXXX This should be sent to the window interface. For now we short-circuit
	// it ourselves.
	m_source->need_redraw(r);
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
	lib::logger::get_logger()->trace("active_region.done(0x%x, \"%s\")", (void *)this, m_source->m_name.c_str());
	need_redraw();
}
