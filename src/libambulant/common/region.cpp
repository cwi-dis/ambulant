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
#include "ambulant/common/preferences.h"

//#define AM_DBG
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
	common::bgrenderer *bgrend,
	common::window_factory *wf)
{
	lib::size bounds = lib::size(common::default_layout_width, common::default_layout_height);
	if (info) {
		lib::basic_rect<int, int> rect = info->get_rect();
		bounds = lib::size(rect.width(), rect.height());
	}
	return new passive_root_layout(info, bounds, bgrend, wf);
}


passive_region::passive_region(const std::string &name, passive_region *parent, screen_rect<int> bounds,
	const region_info *info, bgrenderer *bgrenderer)
:	m_name(name),
	m_bounds_inited(true),
	m_inner_bounds(bounds.innercoordinates(bounds)),
	m_outer_bounds(bounds),
	m_window_topleft(bounds.left_top()),
	m_parent(parent),
	m_info(info),
//	m_alignment(NULL),
	m_bg_renderer(bgrenderer)
{
	if (parent) m_window_topleft += parent->get_global_topleft();
	if (m_bg_renderer) {
		m_bg_renderer->set_surface(this);
	}
}

passive_region::~passive_region()
{
	AM_DBG lib::logger::get_logger()->trace("~passive_region(0x%x)", (void*)this);
	m_parent = NULL;
	std::list<gui_events*>::reverse_iterator ari;
	for(ari=m_renderers.rbegin(); ari!=m_renderers.rend(); ari++) {
		delete (*ari);
	}
	if (m_bg_renderer)
		delete m_bg_renderer;
//	if (m_info)
//		delete m_info;
	for(children_map_t::iterator it1=m_active_children.begin();it1!=m_active_children.end();it1++) {
		children_list_t& cl = (*it1).second;
		for(children_list_t::iterator it2=cl.begin();it2!=cl.end();it2++)
			delete (*it2);
	}
}

common::surface_template *
passive_region::new_subsurface(const region_info *info, bgrenderer *bgrenderer)
{
	screen_rect<int> bounds = info->get_screen_rect();
	zindex_t z = info->get_zindex();
	AM_DBG lib::logger::get_logger()->trace("subbregion %s: ltrb=(%d, %d, %d, %d), z=%d", info->get_name().c_str(), bounds.left(), bounds.top(), bounds.right(), bounds.bottom(), z);
	passive_region *rv = new passive_region(info->get_name(), this, bounds, info, bgrenderer);
	AM_DBG lib::logger::get_logger()->trace("subbregion: returning 0x%x", (void*)rv);
	m_active_children[zindex_t(z)].push_back(rv);
	need_redraw(bounds);
	return rv;
}

common::surface *
passive_region::activate()
{
	return this;
}

void
passive_region::animated()
{
	/*AM_DBG*/ lib::logger::get_logger()->trace("passive_region::animated(%s, 0x%x)", m_name.c_str(), (void*)this);
	clear_cache();
	need_redraw(m_inner_bounds);
	
}

void
passive_region::show(gui_events *cur)
{

	m_renderers.push_back(cur);
	AM_DBG lib::logger::get_logger()->trace("passive_region.show(0x%x, active=0x%x)", (void *)this, (void *)cur);
	// We don't schedule a redraw here, assuming it will come shortly.
	// is that correct?
	
	if(m_parent) {
		children_map_t& subregions =  m_parent->get_subregions();
		subregions[m_info->get_zindex()].push_back(this);
	}
	need_redraw();
}

void
passive_region::renderer_done(gui_events *cur)
{
	AM_DBG lib::logger::get_logger()->trace("passive_region.renderer_done(0x%x, cur=0x%x)", (void *)this, (void*)cur);
	
	std::list<gui_events*>::iterator i = m_renderers.end();
	for(i=m_renderers.begin(); i!=m_renderers.end(); i++)
		if ((*i) == cur) break;
	if (i == m_renderers.end()) {
		lib::logger::get_logger()->error("passive_region.renderer_done(0x%x, 0x%x): not active!", (void *)this, (void*)cur);
	} else {
		m_renderers.erase(i);
	}
	
	if(m_parent) {
		children_map_t& subregions =  m_parent->get_subregions();
		subregions[m_info->get_zindex()].remove(this);
	}

	need_redraw(m_inner_bounds);
}

void
passive_region::redraw(const lib::screen_rect<int> &r, gui_window *window)
{
	AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x %s, ltrb=(%d, %d, %d, %d))", (void *)this, m_name.c_str(), r.left(), r.top(), r.right(), r.bottom());
	screen_rect<int> our_outer_rect = r & m_outer_bounds;
	screen_rect<int> our_rect = m_outer_bounds.innercoordinates(our_outer_rect);
	if (our_rect.empty()) {
	AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x %s) returning: nothing to draw", (void *)this, m_name.c_str());
		return;
	}
	// For now: if we are going to redraw anything we have to redraw everything (sigh).
	our_rect = m_inner_bounds;
	
	////////////////
	// Draw the content of this
	AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x %s, our_ltrb=(%d, %d, %d, %d)) ->draw_background", (void *)this, m_name.c_str(), our_rect.left(), our_rect.top(), our_rect.right(), our_rect.bottom());
	
	// First the background
	draw_background(our_rect, window);	
	
	// Then the active renderers
	// For the win32 arrangement we should have at most one active
	assert(m_renderers.size()<=1);
	std::list<gui_events*>::iterator ar;
	for (ar=m_renderers.begin(); ar!=m_renderers.end(); ar++) {
		AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x %s) ->renderer 0x%x", (void *)this, m_name.c_str(), (void *)(*ar));
		(*ar)->redraw(our_rect, window);
	}
		
	// Draw active subregions in reverse activation order and in the correct z-order
	for(children_map_t::iterator it1=m_subregions.begin();it1!=m_subregions.end();it1++) {
		children_list_t& cl = (*it1).second;
		for(children_list_t::iterator it2=cl.begin();it2!=cl.end();it2++) {
			
			AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x %s) ->subregion 0x%x", (void *)this, m_name.c_str(), (void *)(*it2));
			if (!(*it2)->get_info()->is_subregion()) {
				AM_DBG lib::logger::get_logger()->warn("passive_region.redraw(0x%x): subregion 0x%x is not a subregion", (void*)this, (void*)(*it2));
			}
			(*it2)->redraw(our_rect, window);
		}
	}
	
	// Finally the children regions of this
	// XXXX Should go per z-order value
	for(children_map_t::iterator it2=m_active_children.begin();it2!=m_active_children.end();it2++) {
		AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x %s) examining next z-order list", (void*)this, m_name.c_str());
		children_list_t& cl = (*it2).second;
		for(children_list_t::iterator it3=cl.begin();it3!=cl.end();it3++) {
			if(!(*it3)->get_info()->is_subregion()) {
				AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x %s) -> child 0x%x", (void *)this, m_name.c_str(), (void *)(*it3));
				(*it3)->redraw(our_rect, window);
			}
		}
	}
	AM_DBG lib::logger::get_logger()->trace("passive_region.redraw(0x%x %s) returning", (void*)this, m_name.c_str());
}

void
passive_region::draw_background(const lib::screen_rect<int> &r, gui_window *window)
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
	AM_DBG lib::logger::get_logger()->trace("draw_background(0x%x %s): drawing background", (void*)this, m_name.c_str());
	m_bg_renderer->redraw(r, window);
}

void
passive_region::user_event(const lib::point &where, int what)
{
	AM_DBG lib::logger::get_logger()->trace("passive_region.user_event(0x%x, (%d, %d))", (void *)this, where.x, where.y);
	// Test that it is in our area
	if (!m_outer_bounds.contains(where)) {
		AM_DBG lib::logger::get_logger()->trace("passive_region.user_event: not in our bounds");
		return;
	}
	// Convert to local coordinates
	point our_point = where;
	our_point -= m_outer_bounds.left_top();
	
	std::list<gui_events*>::reverse_iterator ari;
	for (ari=m_renderers.rbegin(); ari!=m_renderers.rend(); ari++) {
		AM_DBG lib::logger::get_logger()->trace("passive_region.user_event(0x%x) ->active 0x%x", (void *)this, (void *)(*ari));
		(*ari)->user_event(our_point, what);
	}
	children_map_t::reverse_iterator it1;
	for(it1=m_active_children.rbegin();it1!=m_active_children.rend();it1++) {
		children_list_t& cl = (*it1).second;
		children_list_t::iterator it2;
		for(it2=cl.begin();it2!=cl.end();it2++) {
			AM_DBG lib::logger::get_logger()->trace("passive_region.user_event(0x%x) -> child 0x%x,z=%d", (void *)this, (void *)(*it2), (*it1).first);
			(*it2)->user_event(our_point, what);
		}
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
passive_region::need_redraw()
{
	need_redraw(m_inner_bounds);
}

void
passive_region::need_events(bool want)
{
	if (!m_parent)
		return;   // Audio region or some such
	// Not good enough: we only forward that we want events
	if (want) m_parent->need_events(want);
}

const lib::point &
passive_region::get_global_topleft() const
{
	const_cast<passive_region*>(this)->need_bounds();
	return m_window_topleft;
}

void
passive_region::need_bounds()
{
	if (m_bounds_inited) return;
	/*AM_DBG*/ lib::logger::get_logger()->trace("passive_region::need_bounds(%s, 0x%x)", m_name.c_str(), (void*)this);
	if (m_info) m_outer_bounds = m_info->get_screen_rect();
	/*AM_DBG*/ lib::logger::get_logger()->trace("passive_region::need_bounds: %d %d %d %d", 
		m_outer_bounds.m_left, m_outer_bounds.m_top, m_outer_bounds.m_right, m_outer_bounds.m_bottom);
	m_inner_bounds = m_outer_bounds.innercoordinates(m_outer_bounds);
	m_window_topleft = m_outer_bounds.left_top();
	if (m_parent) m_window_topleft += m_parent->get_global_topleft();
	m_bounds_inited = true;
}

void
passive_region::clear_cache()
{
	/*AM_DBG*/ lib::logger::get_logger()->trace("passive_region::clear_cache(%s, 0x%x)", m_name.c_str(), (void*)this);
	m_bounds_inited = false;
	// Better be safe than sorry: clear caches for all child regions
	for(children_map_t::iterator it1=m_active_children.begin();it1!=m_active_children.end();it1++) {
		children_list_t& cl = (*it1).second;
		for(children_list_t::iterator it2=cl.begin();it2!=cl.end();it2++)
			(*it2)->clear_cache();
	}
	// XXXX note that this does *not* propagate the change to the
	// renderers yet: if they cache information they will have to
	// invalidate it on the next redraw.
}


lib::screen_rect<int> 
passive_region::get_fit_rect_noalign(const lib::size& src_size, lib::rect* out_src_rect) const
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
passive_region::get_fit_rect(const lib::size& src_size, lib::rect* out_src_rect, common::alignment *align) const
{
	if (align == NULL)
		return get_fit_rect_noalign(src_size, out_src_rect);
		
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
	
	AM_DBG lib::logger::get_logger()->trace("get_fit_rect: image size=(%d, %d)", image_width, image_height);
	AM_DBG lib::logger::get_logger()->trace("get_fit_rect: region size=(%d, %d)", region_width, region_height);
	AM_DBG lib::logger::get_logger()->trace("get_fit_rect: image fixpoint=(%d, %d)", xy_image.x, xy_image.y);
	AM_DBG lib::logger::get_logger()->trace("get_fit_rect: region fixpoint=(%d, %d)", xy_region.x, xy_region.y);
	AM_DBG lib::logger::get_logger()->trace("get_fit_rect: image delta fixpoint to ltrb=(%d, %d, %d, %d)", x_image_left, y_image_top, x_image_right, y_image_bottom);
	AM_DBG lib::logger::get_logger()->trace("get_fit_rect: region delta fixpoint to ltrb=(%d, %d, %d, %d)", x_region_left, y_region_top, x_region_right, y_region_bottom);
	
	double scale_min_horizontal, scale_max_horizontal, scale_min_vertical, scale_max_vertical;
	
	if (x_image_left == 0) {
		scale_min_horizontal = scale_max_horizontal = (double)x_region_right / (double)x_image_right;
	} else {
		scale_min_horizontal = scale_max_horizontal = (double)x_region_left / (double)x_image_left;
		if (x_image_right != 0) {
			double scale_right = (double)x_region_right / (double)x_image_right;
			if (scale_min_horizontal == 0)
				scale_min_horizontal = scale_right;
			else
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
			if (scale_min_vertical == 0)
				scale_min_vertical = scale_bottom;
			else
				scale_min_vertical = std::min(scale_min_vertical, scale_bottom);
			scale_max_vertical = std::max(scale_max_vertical, scale_bottom);
		}
	}
	double scale_horizontal, scale_vertical;
	
	const fit_t fit = (m_info == NULL?fit_hidden : m_info->get_fit());
	switch (fit) {
	  case fit_fill:
		// Fill the area with the image, ignore aspect ration
		// XXX I don't think this is correct. Or is it?
		scale_horizontal = scale_min_horizontal;
		scale_vertical = scale_min_vertical;
		break;
	  case fit_scroll:		// XXXX incorrect
	  case fit_hidden:
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
	AM_DBG lib::logger::get_logger()->trace("get_fit_rect: scale_hor=%f, scale_vert=%f", scale_horizontal, scale_vertical);
	if (scale_horizontal == 0 || scale_vertical == 0) {
		*out_src_rect = lib::rect(point(0,0), size(0,0));
		return lib::screen_rect<int>(point(0,0), point(0,0));
	}
	// Convert the image fixpoint to scaled coordinates
	int x_image_scaled = (int)((xy_image.x * scale_horizontal) + 0.5);
	int y_image_scaled = (int)((xy_image.y * scale_vertical) + 0.5);
	AM_DBG lib::logger::get_logger()->trace("get_fit_rect: scaled image fixpoint=(%d, %d)", x_image_scaled, y_image_scaled);
	// Find out where image (0, 0) would end up, and similarly for other
	// corners. At this point we still allow negative values or values > max
	int x_region_for_image_left = xy_region.x - x_image_scaled;
	int x_region_for_image_right = x_region_for_image_left + (int)((image_width * scale_horizontal) + 0.5);
	int y_region_for_image_top = xy_region.y - y_image_scaled;
	int y_region_for_image_bottom = y_region_for_image_top + (int)((image_height * scale_vertical) + 0.5);
	int x_image_for_region_left = 0;
	int x_image_for_region_right = image_width;
	int y_image_for_region_top = 0;
	int y_image_for_region_bottom = image_height;
	AM_DBG lib::logger::get_logger()->trace("get_fit_rect: full image would  have region lrtb=(%d, %d, %d, %d)", 
		x_region_for_image_left, y_region_for_image_top, x_region_for_image_right, y_region_for_image_bottom);
	// Finally clamp all values
	if (x_region_for_image_left < 0) {
		x_image_for_region_left = (int)((-x_region_for_image_left / scale_horizontal) + 0.5);
		x_region_for_image_left = 0;
	}
	if (x_region_for_image_right > region_width) { // XXXX Or +1?
		int overshoot = x_region_for_image_right - region_width;
		x_region_for_image_right = region_width;
		x_image_for_region_right = x_image_for_region_right - (int)((overshoot / scale_horizontal) + 0.5);
	}
	if (y_region_for_image_top < 0) {
		y_image_for_region_top = (int)((-y_region_for_image_top / scale_vertical) + 0.5);
		y_region_for_image_top = 0;
	}
	if (y_region_for_image_bottom > region_height) {
		int overshoot = y_region_for_image_bottom - region_height;
		y_region_for_image_bottom = region_height;
		y_image_for_region_bottom = y_image_for_region_bottom - (int)((overshoot / scale_vertical) + 0.5);
	}
	AM_DBG lib::logger::get_logger()->trace("get_fit_rect: image selection ltrb=(%d, %d, %d, %d)", 
		x_image_for_region_left, y_image_for_region_top, x_image_for_region_right, y_image_for_region_bottom);
	AM_DBG lib::logger::get_logger()->trace("get_fit_rect: region selection lrtb=(%d, %d, %d, %d)", 
		x_region_for_image_left, y_region_for_image_top, x_region_for_image_right, y_region_for_image_bottom);
	*out_src_rect = lib::rect(
		point(x_image_for_region_left, y_image_for_region_top),
		size(x_image_for_region_right-x_image_for_region_left, y_image_for_region_bottom-y_image_for_region_top));
	return lib::screen_rect<int>(
		point(x_region_for_image_left, y_region_for_image_top),
		point(x_region_for_image_right, y_region_for_image_bottom));
}

passive_root_layout::passive_root_layout(const region_info *info, lib::size bounds, bgrenderer *bgrenderer, window_factory *wf)
:   passive_region(info?info->get_name():"topLayout", NULL, screen_rect<int>(point(0, 0), bounds), info, bgrenderer)
{
	m_gui_window = wf->new_window(m_name, bounds, this);
	AM_DBG lib::logger::get_logger()->trace("passive_root_layout(0x%x, \"%s\"): window=0x%x", (void *)this, m_name.c_str(), (void *)m_gui_window);
}
		
passive_root_layout::~passive_root_layout()
{
	AM_DBG lib::logger::get_logger()->trace("~passive_root_layout(0x%x)", (void*)this);
	if (m_gui_window)
		delete m_gui_window;
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
passive_root_layout::need_events(bool want)
{
	if (m_gui_window)
		m_gui_window->need_events(want);
	else
		lib::logger::get_logger()->error("passive_root_layout::need_events: m_gui_window == NULL");
}
