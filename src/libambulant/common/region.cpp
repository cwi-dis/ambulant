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

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace common;

// Factories
common::surface_factory *
common::create_smil_surface_factory()
{
	return new smil_surface_factory();
}

common::surface_template *
smil_surface_factory::new_topsurface(
	const common::region_info *info,
	common::renderer *bgrend,
	common::window_factory *wf)
{
	lib::size bounds = lib::size(200, 200);
	if (info) {
		lib::basic_rect<int, int> rect = info->get_rect();
		bounds = lib::size(rect.width(), rect.height());
	}
	return new passive_root_layout(info, bounds, bgrend, wf);
}


passive_region::passive_region(const std::string &name, passive_region *parent, screen_rect<int> bounds,
	const region_info *info, renderer *bgrenderer)
:	m_name(name),
	m_name_str(name.c_str()),
	m_bounds_inited(true),
	m_inner_bounds(bounds.innercoordinates(bounds)),
	m_outer_bounds(bounds),
	m_window_topleft(bounds.left_top()),
	m_parent(parent),
	m_cur_active_region(NULL),
	m_bg_active_region(NULL),
	m_old_active_region(NULL),
	m_mouse_region(NULL),
	m_info(info),
	m_bg_renderer(bgrenderer)
{
	if (parent) m_window_topleft += parent->get_global_topleft();
	if (parent && parent->m_mouse_region) {
		m_mouse_region = parent->m_mouse_region->clone();
		m_mouse_region->clear();
	}
	if (m_bg_renderer) {
		m_bg_active_region = static_cast<active_region *>(activate());
		m_bg_renderer->set_surface(m_bg_active_region);
	}
}

passive_region::~passive_region()
{
	AM_DBG lib::logger::get_logger()->trace("~passive_region(0x%x)", (void*)this);
	m_parent = NULL;
	if (m_cur_active_region) {
		lib::logger::get_logger()->error("~passive_region(0x%x): m_cur_active_region = 0x%x", (void *)this, (void *)m_cur_active_region);
		delete m_cur_active_region;
	}
	m_cur_active_region = NULL;
	if (m_bg_renderer)
		delete m_bg_renderer;
	m_bg_renderer = NULL;
	if (m_bg_active_region)
		delete m_bg_active_region;
	m_bg_active_region = NULL;
//	if (m_mouse_region)
//		delete m_mouse_region;
	m_mouse_region = NULL;
//	if (m_info)
//		delete m_info;
	m_info = NULL;
	std::multimap<zindex_t,passive_region *>::iterator i;
	for(i=m_active_children.begin(); i != m_active_children.end(); i++) {
		delete (*i).second;
	}
}

common::surface_template *
passive_region::new_subsurface(const region_info *info, renderer *bgrenderer)
{
	screen_rect<int> bounds = info->get_screen_rect();
	zindex_t z = info->get_zindex();
	AM_DBG lib::logger::get_logger()->trace("subbregion %s: ltrb=(%d, %d, %d, %d), z=%d", info->get_name().c_str(), bounds.left(), bounds.top(), bounds.right(), bounds.bottom(), z);
	passive_region *rv = new passive_region(info->get_name(), this, bounds, info, bgrenderer);
	
	m_active_children.insert(std::make_pair(zindex_t(z), rv));
	
	need_redraw(bounds);
	return rv;
}

common::surface *
passive_region::activate()
{
	active_region *rv = new active_region(this);
	AM_DBG lib::logger::get_logger()->trace("passive_region::activate(%s, 0x%x) -> 0x%x", m_name.c_str(), (void*)this, (void*)rv);
	return rv;
}

void
passive_region::show(active_region *cur)
{
	if (m_cur_active_region) {
		lib::logger::get_logger()->error("passive_region(0x%x).show(0x%x) but m_cur_active_region=0x%x!", (void*)this, (void*)cur, (void*)m_cur_active_region);
	}
//	if (m_cur_active_region)
//		delete m_cur_active_region;
	m_cur_active_region = cur;
	AM_DBG lib::logger::get_logger()->trace("passive_region.show(0x%x, active=0x%x)", (void *)this, (void *)m_cur_active_region);
	// We don't schedule a redraw here, assuming it will come shortly.
	// is that correct?
}

void
passive_region::active_region_done(active_region *cur)
{
	AM_DBG lib::logger::get_logger()->trace("passive_region.active_region_done(0x%x, cur=0x%x), m_cur_active_region=0x%x", (void *)this, (void*)cur, (void *)m_cur_active_region);
	if (cur == m_cur_active_region) {
		m_cur_active_region = NULL;
	} else {
		lib::logger::get_logger()->error("passive_region(0x%x).active_region_done(0x%x) but m_cur_active_region=0x%x!", (void*)this, (void*)cur, (void*)m_cur_active_region);
	}
	delete cur;
	need_redraw(m_inner_bounds);
}

void
passive_region::redraw(const lib::screen_rect<int> &r, abstract_window *window)
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
passive_region::draw_background(const lib::screen_rect<int> &r, abstract_window *window)
{
	// Do a quick return if we have nothing to draw
	if (m_info == NULL) {
		AM_DBG lib::logger::get_logger()->trace("draw_background %s: no m_info", m_name.c_str());
		return;
	}
	AM_DBG lib::logger::get_logger()->trace("draw_background %s: color=0x%x, transparent=%x, showbg=%d, renderer=0x%x",
		m_name.c_str(), (int)m_info->get_bgcolor(), (int)m_info->get_transparent(), (int)m_info->get_showbackground(), (int)m_bg_renderer);
	if (m_info->get_transparent()) {
		AM_DBG lib::logger::get_logger()->trace("draw_background %s: transparent", m_name.c_str());
		return;
	}
	if (!m_info->get_showbackground()) {
		AM_DBG lib::logger::get_logger()->trace("draw_background %s: showbackground is false", m_name.c_str());
		return;
	}
	// Now we should make sure we have a background renderer
	if (!m_bg_renderer) {
		AM_DBG lib::logger::get_logger()->trace("draw_background %s: no m_bg_renderer", m_name.c_str());
		return;
	}
	m_bg_renderer->redraw(r, window);
}

void
passive_region::user_event(const lib::point &where)
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
passive_region::need_redraw(const lib::screen_rect<int> &r)
{
	if (!m_parent)
		return;   // Audio region or some such
	screen_rect<int> parent_rect = r & m_inner_bounds;
	m_parent->need_redraw(m_outer_bounds.outercoordinates(parent_rect));
}

void
passive_region::need_events(gui_region *rgn)
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
passive_region::get_global_topleft() const
{
	const_cast<passive_region*>(this)->need_bounds();
	return m_window_topleft;
}


lib::screen_rect<int> 
passive_region::get_fit_rect(const lib::size& src_size, lib::rect* out_src_rect) const
{
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
	
	const fit_t fit = (m_info == NULL?fit_hidden : m_info->get_fit());
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

lib::screen_rect<int> 
passive_region::get_fit_rect(const lib::size& src_size, const alignment *align, lib::rect* out_src_rect) const
{
	if (align == NULL)
		return get_fit_rect(src_size, out_src_rect);
		
	const_cast<passive_region*>(this)->need_bounds();
	const int image_width = src_size.w;
	const int image_height = src_size.h;
	const int region_width = m_inner_bounds.width();
	const int region_height = m_inner_bounds.height();
	lib::size region_size = lib::size(region_width, region_height);
	
	lib::point xy_image = align->get_image_fixpoint(src_size);
	lib::point xy_region = align->get_surface_fixpoint(region_size);
	
	const int x_image_left = xy_image.x;
	const int x_image_right = image_width - xy_image.x;
	const int y_image_top = xy_image.y;
	const int y_image_bottom = image_height - xy_image.y;
	const int x_region_left = xy_region.x;
	const int x_region_right = region_width - xy_region.x;
	const int y_region_top = xy_region.y;
	const int y_region_bottom = region_height - xy_region.y;
	
	double scale_min_horizontal, scale_max_horizontal, scale_min_vertical, scale_max_vertical;
	
	if (x_image_left == 0) {
		scale_min_horizontal = scale_max_horizontal = (double)x_region_right / (double)x_image_right;
	} else {
		scale_min_horizontal = scale_max_horizontal = (double)x_region_left / (double)x_image_left;
		if (x_image_right != 0) {
			double scale_right = (double)x_region_right / (double)x_image_right;
			scale_min_horizontal = std::min(scale_min_horizontal, scale_right);
			scale_max_horizontal = std::max(scale_max_horizontal, scale_right);
		}
	}
	if (y_image_top == 0) {
		scale_min_vertical = scale_max_vertical = (double)y_region_bottom / (double)y_image_bottom;
	} else {
		scale_min_vertical = scale_max_vertical = (double)y_region_top / (double)y_image_top;
		if (y_image_bottom != 0) {
			double scale_bottom = (double)y_region_bottom / (double)y_image_bottom;
			scale_min_vertical = std::min(scale_min_vertical, scale_bottom);
			scale_max_vertical = std::max(scale_max_vertical, scale_bottom);
		}
	}
	double scale_horizontal, scale_vertical;
	
	const fit_t fit = (m_info == NULL?fit_hidden : m_info->get_fit());
	switch (fit) {
	  case fit_fill:
		// Fill the area with the image, ignore aspect ration
		// XXX I don't think this is correct
		scale_horizontal = scale_min_horizontal;
		scale_vertical = scale_min_vertical;
		break;
	  case fit_scroll:
	  case fit_hidden:
		// Don't scale at all
		// XXXX incorrect
		scale_horizontal = scale_vertical = 1.0;
		break;
	  case fit_meet:
		// Scale to make smallest edge fit (showing some background color)
		scale_horizontal = scale_vertical = std::min(scale_min_horizontal, scale_min_vertical);
		break;
	  case fit_slice:
		// Scale to make largest edge fit (not showing the full source image)
		scale_horizontal = scale_vertical = std::max(scale_max_horizontal, scale_max_vertical);
		break;
	}
	// XXX There's more to be done here...
	// Just so we don't crash we call the non-regpoint routine
	/*AM_DBG*/lib::logger::get_logger()->trace("get_fit_rect: scale_h=%f, scale_v=%f", scale_horizontal, scale_vertical);
	lib::logger::get_logger()->error("regPoint/regAlign not implemented yet");
	return get_fit_rect(src_size, out_src_rect);
}

void
passive_region::need_bounds()
{
	if (m_bounds_inited) return;
	if (m_info) m_outer_bounds = m_info->get_screen_rect();
	m_inner_bounds = m_outer_bounds.innercoordinates(m_outer_bounds);
	m_window_topleft = m_outer_bounds.left_top();
	if (m_parent) m_window_topleft += m_parent->get_global_topleft();
	m_bounds_inited = true;
}

void
passive_region::clear_cache()
{
	m_bounds_inited = false;
}

void
passive_region::mouse_region_changed()
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

passive_root_layout::passive_root_layout(const region_info *info, lib::size bounds, renderer *bgrenderer, window_factory *wf)
:   passive_region(info?info->get_name():"topLayout", NULL, screen_rect<int>(point(0, 0), bounds), info, bgrenderer)
{
	m_mouse_region = wf->new_mouse_region();
	m_gui_window = wf->new_window(m_name, bounds, this);
	AM_DBG lib::logger::get_logger()->trace("passive_root_layout(0x%x, \"%s\"): window=0x%x, mouse_region=0x%x", (void *)this, m_name.c_str(), (void *)m_gui_window, (void *)m_mouse_region);
}
		
passive_root_layout::~passive_root_layout()
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
passive_root_layout::need_redraw(const lib::screen_rect<int> &r)
{
	if (m_gui_window)
		m_gui_window->need_redraw(r);
	else
		lib::logger::get_logger()->error("passive_root_layout::need_redraw: m_gui_window == NULL");
}

void
passive_root_layout::mouse_region_changed()
{
	passive_region::mouse_region_changed();
	if (m_gui_window)
		m_gui_window->mouse_region_changed();
	else
		lib::logger::get_logger()->error("passive_root_layout::mouse_region_changed: m_gui_window == NULL");
}

active_region::~active_region()
{
	AM_DBG lib::logger::get_logger()->trace("active_region::~active_region(0x%x)", (void*)this);
	if (m_mouse_region) delete m_mouse_region;
	if (m_renderer) lib::logger::get_logger()->warn("active_region::~active_region(0x%x): m_renderer=0x%x", (void*)this, (void*)m_renderer);
}

void
active_region::show(renderer *rend)
{
	m_renderer = rend;
	m_source->show(this);
	AM_DBG lib::logger::get_logger()->trace("active_region.show(0x%x, \"%s\", renderer=0x%x)", (void *)this, m_source->m_name.c_str(), (void *)rend);
	need_redraw();
}

void
active_region::redraw(const lib::screen_rect<int> &r, abstract_window *window)
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
active_region::user_event(const lib::point &where)
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
active_region::need_redraw(const lib::screen_rect<int> &r)
{
	m_source->need_redraw(r);
}

void
active_region::need_redraw()
{
	need_redraw(m_source->m_inner_bounds);
}

void
active_region::need_events(bool want)
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
active_region::renderer_done()
{
	m_renderer = NULL;
	AM_DBG lib::logger::get_logger()->trace("active_region.done(0x%x, \"%s\")", (void *)this, m_source->m_name.c_str());
	need_events(false);
	// Note: whether we are deleted or not is up to our passive_region
	// parent: it may want to keep us around for a transition or some such.
	m_source->active_region_done(this);
}
