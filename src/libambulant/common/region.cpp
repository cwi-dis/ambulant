/* 
 * @$Id$ 
 */

#include "ambulant/lib/logger.h"
#include "ambulant/common/region.h"
#include "ambulant/common/renderer.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

lib::passive_region *
lib::passive_region::subregion(const std::string &name, screen_rect<int> bounds)
{
	point topleft = m_window_topleft + bounds.left_top();
	passive_region *rv = new passive_region(name, this, bounds, topleft);
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
	AM_DBG lib::logger::get_logger()->trace("passive_region.show(0x%x, active=0x%x)", (void *)this, (void *)m_cur_active_region);
}

void
lib::passive_region::redraw(const screen_rect<int> &r, passive_window *window)
{
	AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x, ltrb=(%d, %d, %d, %d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	screen_rect<int> our_outer_rect = r & m_outer_bounds;
	screen_rect<int> our_rect = m_outer_bounds.innercoordinates(our_outer_rect);
	if (our_rect.empty())
		return;
	AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x, our_ltrb=(%d, %d, %d, %d))", (void *)this, our_rect.left(), our_rect.top(), our_rect.right(), our_rect.bottom());
		
	if (m_cur_active_region) {
		AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x) ->active 0x%x", (void *)this, (void *)m_cur_active_region);
		m_cur_active_region->redraw(our_rect, window, m_window_topleft);
	} else {
		AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x) no active region", (void *)this);
	}
	std::vector<passive_region *>::iterator i;
	for(i=m_children.begin(); i<m_children.end(); i++) {
		AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x) -> child 0x%x", (void *)this, (void *)(*i));
		(*i)->redraw(our_rect, window);
	}
}

void
lib::passive_region::need_redraw(const screen_rect<int> &r)
{
	if (!m_parent)
		return;   // Audio region or some such
	screen_rect<int> parent_rect = r & m_inner_bounds;
	m_parent->need_redraw(m_outer_bounds.outercoordinates(parent_rect));
}

void
lib::passive_window::need_redraw(const screen_rect<int> &r)
{
	AM_DBG lib::logger::get_logger()->trace("passive_window.need_redraw(0x%x)", (void *)this);
	// XXXX This should be sent to the window interface. For now we short-circuit
	// it ourselves.
	screen_rect<int> redrawregion = r & m_inner_bounds;
	redraw(redrawregion, this);
}

void
lib::active_region::show(active_renderer *renderer)
{
	m_renderer = renderer;
	m_source->show(this);
	AM_DBG lib::logger::get_logger()->trace("active_region.show(0x%x, \"%s\", renderer=0x%x)", (void *)this, m_source->m_name.c_str(), (void *)renderer);
	need_redraw();
}

void
lib::active_region::redraw(const screen_rect<int> &r, passive_window *window, const point &window_topleft)
{
	if (m_renderer) {
		AM_DBG lib::logger::get_logger()->trace("active_region.redraw(0x%x) -> renderer 0x%x", (void *)this, (void *)m_renderer);
		m_renderer->redraw(r, window, window_topleft);
	} else {
		// At this point we should have a renderer that draws the default background
		// When that is implemented this trace message should turn into an error (or fatal).
		AM_DBG lib::logger::get_logger()->trace("active_region.redraw(0x%x) no renderer", (void *)this);
	}
}

void
lib::active_region::need_redraw(const screen_rect<int> &r)
{
	m_source->need_redraw(r);
}

void
lib::active_region::need_redraw()
{
	need_redraw(m_source->m_inner_bounds);
}

void
lib::active_region::done()
{
	m_renderer = NULL;
	AM_DBG lib::logger::get_logger()->trace("active_region.done(0x%x, \"%s\")", (void *)this, m_source->m_name.c_str());
	need_redraw();
}
