/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

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

common::passive_region::~passive_region()
{
	AM_DBG lib::logger::get_logger()->trace("~passive_region(0x%x)", (void*)this);
	m_parent = NULL;
	if (m_cur_active_region) {
		lib::logger::get_logger()->error("~passive_region(0x%x): m_cur_active_region = 0x%x", (void *)this, (void *)m_cur_active_region);
		delete m_cur_active_region;
	}
	m_cur_active_region = NULL;
//	if (m_mouse_region)
//		delete m_mouse_region;
	m_mouse_region = NULL;
//	if (m_info)
//		delete m_info;
	m_info = NULL;
	// Don't touch m_bg_renderer: it is shared and destroyed by the root window
	std::multimap<zindex_t,passive_region *>::iterator i;
	for(i=m_active_children.begin(); i != m_active_children.end(); i++) {
		delete (*i).second;
	}
}
	
common::passive_region *
common::passive_region::subregion(const std::string &name, screen_rect<int> bounds)
{
	AM_DBG lib::logger::get_logger()->trace("subbregion NO-INFO: ltrb=(%d, %d, %d, %d)", bounds.left(), bounds.top(), bounds.right(), bounds.bottom());
	passive_region *rv = new passive_region(name, this, bounds, NULL);
	m_active_children.insert(std::make_pair(zindex_t(0), rv));
	return rv;
}

common::passive_region *
common::passive_region::subregion(const abstract_smil_region_info *info)
{
	screen_rect<int> bounds = info->get_screen_rect();
	zindex_t z = info->get_zindex();
	AM_DBG lib::logger::get_logger()->trace("subbregion %s: ltrb=(%d, %d, %d, %d), z=%d", info->get_name().c_str(), bounds.left(), bounds.top(), bounds.right(), bounds.bottom(), z);
	passive_region *rv = new passive_region(info->get_name(), this, bounds, info);
	
	m_active_children.insert(std::make_pair(zindex_t(z), rv));
	return rv;
}

common::active_region *
common::passive_region::activate(const node *node)
{
	active_region *rv = new common::active_region(this, node);
	AM_DBG lib::logger::get_logger()->trace("passive_region::activate(%s, 0x%x) -> 0x%x", m_name.c_str(), (void*)this, (void*)rv);
	return rv;
}

void
common::passive_region::show(active_region *cur)
{
	if (m_cur_active_region) {
		lib::logger::get_logger()->error("passive_region(0x%x).show(0x%x) but m_cur_active_region=0x%x!", (void*)this, (void*)cur, (void*)m_cur_active_region);
	}
//	if (m_cur_active_region)
//		delete m_cur_active_region;
	m_cur_active_region = cur;
	AM_DBG lib::logger::get_logger()->trace("passive_region.show(0x%x, active=0x%x)", (void *)this, (void *)m_cur_active_region);
}

void
common::passive_region::active_region_done()
{
	if (!m_cur_active_region) {
		lib::logger::get_logger()->error("passive_region(0x%x).active_region_done() but m_cur_active_region=0x%x!", (void*)this, (void*)m_cur_active_region);
	}
	delete m_cur_active_region;
	m_cur_active_region = NULL;
}

void
common::passive_region::redraw(const screen_rect<int> &r, abstract_window *window)
{
	AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x, ltrb=(%d, %d, %d, %d))", (void *)this, r.left(), r.top(), r.right(), r.bottom());
	screen_rect<int> our_outer_rect = r & m_outer_bounds;
	screen_rect<int> our_rect = m_outer_bounds.innercoordinates(our_outer_rect);
	if (our_rect.empty())
		return;
	AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x, our_ltrb=(%d, %d, %d, %d))", (void *)this, our_rect.left(), our_rect.top(), our_rect.right(), our_rect.bottom());
		
	if (m_cur_active_region) {
		AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x) ->active 0x%x", (void *)this, (void *)m_cur_active_region);
		m_cur_active_region->redraw(our_rect, window);
	} else {
		AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x) no active region", (void *)this);
		draw_background(our_rect, window);
	}
	// XXXX Should go per z-order value
	std::multimap<zindex_t,passive_region *>::iterator i;
	for(i=m_active_children.begin(); i != m_active_children.end(); i++) {
		AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x) -> child 0x%x, z=%d", (void *)this, (void *)(*i).second, (*i).first);
		(*i).second->redraw(our_rect, window);
	}
}

void
common::passive_region::draw_background(const screen_rect<int> &r, abstract_window *window)
{
	// Do a quick return if we have nothing to draw
	if (m_info == NULL) return;
	AM_DBG lib::logger::get_logger()->trace("draw_background %s: color=0x%x, transparent=%x, showbg=%d, renderer=0x%x",
		m_name.c_str(), (int)m_info->get_bgcolor(), (int)m_info->get_transparent(), (int)m_info->get_showbackground(), (int)m_bg_renderer);
	if (m_info->get_transparent()) return;
	if (!m_info->get_showbackground()) return;
	// Now we should make sure we have a background renderer
	get_bg_renderer();
	if (m_bg_renderer)
		m_bg_renderer->drawbackground(m_info, r, this, window);
}

common::abstract_bg_rendering_source *
common::passive_region::get_bg_renderer()
{
	if (!m_bg_renderer && m_parent)
		m_bg_renderer = m_parent->get_bg_renderer();
	return m_bg_renderer;
}

void
common::passive_region::user_event(const point &where)
{
	AM_DBG lib::logger::get_logger()->trace("passive_region.user_event(0x%x, (%d, %d))", (void *)this, where.x, where.y);
	// Test that it is in our area
	if (!m_outer_bounds.contains(where)) {
		AM_DBG lib::logger::get_logger()->trace("passive_region.user_event: not in our bounds");
		return;
	}
	// And test whether it is in our mouse area too
	if (!m_mouse_region->contains(where)) {
		AM_DBG lib::logger::get_logger()->trace("passive_region.user_event: not in our mouse region");
		return;
	}
	// Convert to local coordinates
	point our_point = where;
	our_point -= m_outer_bounds.left_top();
	
	if (m_cur_active_region) {
		AM_DBG lib::logger::get_logger()->trace("passive_region.user_event(0x%x) ->active 0x%x", (void *)this, (void *)m_cur_active_region);
		m_cur_active_region->user_event(our_point);
	} else {
		AM_DBG lib::logger::get_logger()->error("passive_region.user_event(0x%x) no active region", (void *)this);
	}
	std::multimap<zindex_t,passive_region *>::iterator i;
	for(i=m_active_children.begin(); i != m_active_children.end(); i++) {
		AM_DBG lib::logger::get_logger()->trace("passive_region.user_event(0x%x) -> child 0x%x,z=%d", (void *)this, (void *)(*i).second, (*i).first);
		(*i).second->user_event(our_point);
	}
}

void
common::passive_region::need_redraw(const screen_rect<int> &r)
{
	if (!m_parent)
		return;   // Audio region or some such
	screen_rect<int> parent_rect = r & m_inner_bounds;
	m_parent->need_redraw(m_outer_bounds.outercoordinates(parent_rect));
}

void
common::passive_region::need_events(abstract_mouse_region *rgn)
{
	if (!m_parent)
		return;   // Audio region or some such
	// Clip to our inner rect
        *rgn &= m_inner_bounds;
        // Convert to parent coordinate space
        *rgn += m_outer_bounds.left_top();
        if (m_mouse_region) delete m_mouse_region;
        m_mouse_region = rgn;
        m_parent->mouse_region_changed();
}

const lib::point &
common::passive_region::get_global_topleft() const
{
	const_cast<passive_region*>(this)->need_bounds();
	return m_window_topleft;
}


lib::screen_rect<int> 
common::passive_region::get_fit_rect(const lib::size& src_size, lib::rect* out_src_rect) const
{
	// XXXX For now we implement fit=fill only
	const_cast<passive_region*>(this)->need_bounds();
	const int image_width = src_size.w;
	const int image_height = src_size.h;
	const int region_width = m_inner_bounds.width();
	const int region_height = m_inner_bounds.height();
	const int min_width = std::min(image_width, region_width);
	const int min_height = std::min(image_height, region_height);
	const double scale_width = (double)region_width / std::max((double)image_width, 0.1);
	const double scale_height = (double)region_height / std::max((double)image_height, 0.1);
	double scale;
	
	const common::fit_t fit = (m_info == NULL? common::fit_hidden : m_info->get_fit());
	switch (fit) {
	  case fit_fill:
		// Fill the area with the image, ignore aspect ration
		*out_src_rect = lib::rect(lib::point(0, 0), src_size);
		return m_inner_bounds;
	  case fit_scroll:
	  case fit_hidden:
		// Don't scale at all
		*out_src_rect = lib::rect(lib::point(0, 0), lib::size(min_width, min_height));
		return screen_rect<int>(lib::point(0, 0), lib::point(min_width, min_height));
	  case fit_meet:
		// Scale to make smallest edge fit (showing some background color)
		scale = std::min(scale_width, scale_height);
		break;
	  case fit_slice:
		// Scale to make largest edge fit (not showing the full source image)
		scale = std::max(scale_width, scale_height);
		break;
	}
	// We end up here as common case for meet and slice
	int proposed_width = std::min((int)(scale*(image_width+0.5)), region_width);
	int proposed_height = std::min((int)(scale*(image_height+0.5)), region_height);
	*out_src_rect = lib::rect(lib::point(0, 0), lib::size((int)(proposed_width/scale), (int)(proposed_height/scale)));
	return screen_rect<int>(lib::point(0, 0), lib::point(proposed_width, proposed_height));
}

void
common::passive_region::need_bounds()
{
	if (m_bounds_inited) return;
	if (m_info) m_outer_bounds = m_info->get_screen_rect();
	m_inner_bounds = m_outer_bounds.innercoordinates(m_outer_bounds);
	m_window_topleft = m_outer_bounds.left_top();
	if (m_parent) m_window_topleft += m_parent->get_global_topleft();
	m_bounds_inited = true;
}

void
common::passive_region::clear_cache()
{
	m_bounds_inited = false;
}

void
common::passive_region::mouse_region_changed()
{
    // Check that we have a mouse region and a parent
    if (!m_mouse_region || !m_parent) {
	      lib::logger::get_logger()->warn("mouse_region_changed: region %s is not a visual region", m_name.c_str());
	      return;
    }
    // Clear the mouse region
    m_mouse_region->clear();
    // Fill with the union of the regions of all our children
    if (m_cur_active_region)
        *m_mouse_region |= m_cur_active_region->get_mouse_region();
    std::multimap<zindex_t,passive_region *>::iterator i;
    for(i=m_active_children.begin(); i != m_active_children.end(); i++) {
        *m_mouse_region |= (*i).second->get_mouse_region();
    }
    // Convert to our parent coordinate space
    *m_mouse_region += m_outer_bounds.left_top();
    // Tell our parent, if we have one
	AM_DBG lib::logger::get_logger()->trace("mouse_region_changed(0x%x): is_empty()=%d", (void*)this, (int)m_mouse_region->is_empty());
    if (m_parent) m_parent->mouse_region_changed();
}

common::passive_root_layout::passive_root_layout(const std::string &name, size bounds, window_factory *wf)
:   passive_region(name, NULL, screen_rect<int>(point(0, 0), bounds), NULL)
{
	m_mouse_region = wf->new_mouse_region();
	m_gui_window = wf->new_window(name, bounds, this);
	m_bg_renderer = wf->new_background_renderer();
	AM_DBG lib::logger::get_logger()->trace("passive_root_layout(0x%x, \"%s\"): window=0x%x, mouse_region=0x%x, bgrenderer=0x%x", (void *)this, m_name.c_str(), (void *)m_gui_window, (void *)m_mouse_region, (void *)m_bg_renderer);
}
		
common::passive_root_layout::passive_root_layout(const abstract_smil_region_info *info, size bounds, window_factory *wf)
:   passive_region(info?info->get_name():"topLayout", NULL, screen_rect<int>(point(0, 0), bounds), info)
{
	m_mouse_region = wf->new_mouse_region();
	m_gui_window = wf->new_window(m_name, bounds, this);
	m_bg_renderer = wf->new_background_renderer();
	AM_DBG lib::logger::get_logger()->trace("passive_root_layout(0x%x, \"%s\"): window=0x%x, mouse_region=0x%x", (void *)this, m_name.c_str(), (void *)m_gui_window, (void *)m_mouse_region);
}
		
common::passive_root_layout::~passive_root_layout()
{
	AM_DBG lib::logger::get_logger()->trace("~passive_root_layout(0x%x)", (void*)this);
	if (m_bg_renderer)
		delete m_bg_renderer;
	m_bg_renderer = NULL;
	if (m_gui_window)
		delete m_gui_window;
	m_gui_window = NULL;
}

void
common::passive_root_layout::need_redraw(const screen_rect<int> &r)
{
	if (m_gui_window)
		m_gui_window->need_redraw(r);
	else
		lib::logger::get_logger()->error("passive_root_layout::need_redraw: m_gui_window == NULL");
}

void
common::passive_root_layout::mouse_region_changed()
{
	common::passive_region::mouse_region_changed();
	if (m_gui_window)
		m_gui_window->mouse_region_changed();
	else
		lib::logger::get_logger()->error("passive_root_layout::mouse_region_changed: m_gui_window == NULL");
}

common::active_region::~active_region()
{
	AM_DBG lib::logger::get_logger()->trace("active_region::~active_region(0x%x)", (void*)this);
	if (m_mouse_region) delete m_mouse_region;
	if (m_renderer) lib::logger::get_logger()->warn("active_region::~active_region(0x%x): m_renderer=0x%x", (void*)this, (void*)m_renderer);
}

void
common::active_region::show(abstract_rendering_source *renderer)
{
	m_renderer = renderer;
	m_source->show(this);
	AM_DBG lib::logger::get_logger()->trace("active_region.show(0x%x, \"%s\", renderer=0x%x)", (void *)this, m_source->m_name.c_str(), (void *)renderer);
	need_redraw();
}

void
common::active_region::redraw(const screen_rect<int> &r, abstract_window *window)
{
	if (m_renderer) {
		AM_DBG lib::logger::get_logger()->trace("active_region.redraw(0x%x) -> renderer 0x%x", (void *)this, (void *)m_renderer);
		m_renderer->redraw(r, window);
	} else {
		// At this point we should have a renderer that draws the default background
		// When that is implemented this trace message should turn into an error (or fatal).
		AM_DBG lib::logger::get_logger()->trace("active_region.redraw(0x%x) no renderer", (void *)this);
	}
}

void
common::active_region::user_event(const point &where)
{
	if (m_renderer) {
		AM_DBG lib::logger::get_logger()->trace("active_region.user_event(0x%x) -> renderer 0x%x", (void *)this, (void *)m_renderer);
		m_renderer->user_event(where);
	} else {
		// At this point we should have a renderer that draws the default background
		// When that is implemented this trace message should turn into an error (or fatal).
		AM_DBG lib::logger::get_logger()->error("active_region.user_event(0x%x) no renderer", (void *)this);
	}
}

void
common::active_region::need_redraw(const screen_rect<int> &r)
{
	m_source->need_redraw(r);
}

void
common::active_region::need_redraw()
{
	need_redraw(m_source->m_inner_bounds);
}

void
common::active_region::need_events(bool want)
{
	if (!m_mouse_region ) {
		lib::logger::get_logger()->warn("mouse_region_changed: region %s is not a visual region", m_source->m_name.c_str());
		return;
	}
	if (!want && m_mouse_region->is_empty())
		return;
	m_mouse_region->clear();
	if (want) *m_mouse_region = m_source->m_inner_bounds;
	AM_DBG lib::logger::get_logger()->trace("active_region::need_events(%d): is_empty() is %d", (int)want, (int)m_mouse_region->is_empty());
	m_source->mouse_region_changed();
}

void
common::active_region::renderer_done()
{
	m_renderer = NULL;
	AM_DBG lib::logger::get_logger()->trace("active_region.done(0x%x, \"%s\")", (void *)this, m_source->m_name.c_str());
	need_events(false);
	need_redraw();	
	m_source->active_region_done();
}
