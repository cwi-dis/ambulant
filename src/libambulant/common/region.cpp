// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "ambulant/lib/logger.h"
#include "ambulant/common/region.h"
#include "ambulant/common/preferences.h"

//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace common;
using namespace lib;

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
	lib::size bounds(0,0);
	rect default_rect = wf->get_default_size();
	if (info) {
		rect rect = info->get_rect(&default_rect);
		bounds = lib::size(rect.width(), rect.height());
	} else {
		bounds = lib::size(default_rect.width(), default_rect.height());
	}
	return new toplevel_surface_impl(info, bounds, bgrend, wf);
}


surface_impl::surface_impl(const std::string &name, surface_impl *parent, rect bounds,
	const region_info *info, bgrenderer *bgrenderer)
:	m_name(name),
	m_bounds_inited(false),
	m_highlighting(false),
	m_inner_bounds(bounds.innercoordinates(bounds)),
	m_outer_bounds(bounds),
	m_window_topleft(bounds.left_top()),
	m_parent(parent),
	m_info(info),
//	m_alignment(NULL),
	m_bg_renderer(bgrenderer),
	m_renderer_data(NULL),
	m_renderer_id(NULL),
	m_forced_redraw(false)
{
	if (parent) m_window_topleft += parent->get_global_topleft();
	if (m_bg_renderer) {
		m_bg_renderer->set_surface(this);
	}
	if (!m_info) lib::logger::get_logger()->debug("surface_impl(): m_info==NULL for \"%s\"", name.c_str());
}

surface_impl::~surface_impl()
{
	AM_DBG lib::logger::get_logger()->debug("~surface_impl(0x%x)", (void*)this);
	m_parent = NULL;
	std::list<gui_events*>::reverse_iterator ari;
	for(ari=m_renderers.rbegin(); ari!=m_renderers.rend(); ari++) {
		delete (*ari);
	}
	if (m_bg_renderer)
		delete m_bg_renderer;
	m_bg_renderer = NULL;
	if (m_renderer_data)
		m_renderer_data->release();
	m_renderer_data = NULL;
//	if (m_info)
//		delete m_info;
	for(children_map_t::iterator it1=m_active_children.begin();it1!=m_active_children.end();it1++) {
		children_list_t& cl = (*it1).second;
		for(children_list_t::iterator it2=cl.begin();it2!=cl.end();it2++)
			delete (*it2);
	}
}

common::surface_template *
surface_impl::new_subsurface(const region_info *info, bgrenderer *bgrenderer)
{
	rect bounds = info->get_rect();
	zindex_t z = info->get_zindex();
	AM_DBG lib::logger::get_logger()->debug("subbregion %s: ltrb=(%d, %d, %d, %d), z=%d", info->get_name().c_str(), bounds.left(), bounds.top(), bounds.right(), bounds.bottom(), z);
	surface_impl *rv = new surface_impl(info->get_name(), this, bounds, info, bgrenderer);
	AM_DBG lib::logger::get_logger()->debug("subbregion: returning 0x%x", (void*)rv);
	m_children_cs.enter();
	m_active_children[zindex_t(z)].push_back(rv);
	m_children_cs.leave();
	need_redraw(bounds);
	return rv;
}

common::surface *
surface_impl::activate()
{
	return this;
}

void
surface_impl::animated()
{
	AM_DBG lib::logger::get_logger()->debug("surface_impl::animated(%s, 0x%x)", m_name.c_str(), (void*)this);
	lib::rect to_redraw = m_outer_bounds;
	clear_cache();
	need_bounds();
	to_redraw |= m_outer_bounds;
	AM_DBG lib::logger::get_logger()->debug("animated: to_redraw=(%d, %d, %d, %d)", to_redraw.left(), to_redraw.top(), to_redraw.width(), to_redraw.height());
	set_forced_redraw();
	m_parent->need_redraw(to_redraw);
}

void
surface_impl::show(gui_events *cur)
{
	m_children_cs.enter();
	// Sanity check: it shouldn't be in here already
	std::list<gui_events*>::iterator i;
	bool was_inactive = m_renderers.size() == 0;
	for(i=m_renderers.begin(); i!=m_renderers.end(); i++) assert((*i) != cur);

	m_renderers.push_back(cur);
	m_children_cs.leave();
	AM_DBG lib::logger::get_logger()->debug("surface_impl[0x%x].show(0x%x)", (void *)this, (void *)cur);
	zindex_t z = 0;
	if (m_info) z = m_info->get_zindex();
	if(m_parent) {
		m_parent->add_subregion(z, this);
	}
	need_redraw();
	if (was_inactive) background_render_changed();
}

void
surface_impl::renderer_done(gui_events *cur)
{
	AM_DBG lib::logger::get_logger()->debug("surface_impl[0x%x].renderer_done(0x%x)", (void *)this, (void*)cur);

	m_children_cs.enter();
	std::list<gui_events*>::iterator i = m_renderers.end();
	for(i=m_renderers.begin(); i!=m_renderers.end(); i++)
		if ((*i) == cur) break;
	if (i == m_renderers.end()) {
		lib::logger::get_logger()->trace("surface_impl[0x%x].renderer_done(0x%x): not found in %d active renderers!",
			(void *)this, (void*)cur, m_renderers.size());
	} else {
		m_renderers.erase(i);
	}
	m_children_cs.leave();

	zindex_t z = 0;
	if (m_info) z = m_info->get_zindex();
	if(m_parent) {
		m_parent->del_subregion(z, this);
	}

	need_redraw(m_inner_bounds);
	if (m_renderers.size() == 0) background_render_changed();
}

void
surface_impl::keep_as_background()
{
	if (m_info->is_subregion())
		m_parent->keep_as_background();
	else if (m_bg_renderer)
		m_bg_renderer->keep_as_background();
}

void
surface_impl::redraw(const lib::rect &r, gui_window *window)
{
	AM_DBG lib::logger::get_logger()->debug("surface_impl.redraw(0x%x %s, ltrb=(%d, %d, %d, %d))", (void *)this, m_name.c_str(), r.left(), r.top(), r.right(), r.bottom());
	rect our_outer_rect = r & m_outer_bounds;
	rect our_rect = m_outer_bounds.innercoordinates(our_outer_rect);
	if (our_rect.empty() && !m_forced_redraw) {
	AM_DBG lib::logger::get_logger()->debug("surface_impl.redraw(0x%x %s) returning: nothing to draw", (void *)this, m_name.c_str());
		return;
	}
	m_forced_redraw = false;

	////////////////
	// Draw the content of this
	AM_DBG lib::logger::get_logger()->debug("surface_impl.redraw(0x%x %s, our_ltrb=(%d, %d, %d, %d)) ->draw_background", (void *)this, m_name.c_str(), our_rect.left(), our_rect.top(), our_rect.right(), our_rect.bottom());

	// First the background
	//marisa added null check july 7 2008
	const region_info *info = get_info();
	if (info != NULL) {
		// We show the background if it isn't transparent. And then, only if showBackground==always, or
		// if there are active children.
		// Note: not 100% sure this is correct: should we also draw the background if there are
		// subregions with active children?
		bool showbg = !info->get_transparent();
		if (showbg) {
			if (!info->get_showbackground()) {
				if (!_is_active()) {
					showbg = false;
				}
			}
		}
		if (showbg) {
			draw_background(our_rect, window);
		}
	}
	// Then the active renderers
	// For the win32 arrangement we should have at most one active
	m_children_cs.enter();

	//XXX Why is this assertion needed ?
	//assert(m_renderers.size()<=1);

	std::list<gui_events*>::iterator ar;
	for (ar=m_renderers.begin(); ar!=m_renderers.end(); ar++) {
		AM_DBG lib::logger::get_logger()->debug("surface_impl.redraw(0x%x %s) ->renderer 0x%x", (void *)this, m_name.c_str(), (void *)(*ar));
		(*ar)->redraw(our_rect, window);
	}
	zindex_t last_z_index = -1;
	// Draw active subregions in reverse activation order and in the correct z-order
	for(children_map_t::iterator it1=m_subregions.begin();it1!=m_subregions.end();it1++) {
		children_list_t& cl = (*it1).second;
		for(children_list_t::iterator it2=cl.begin();it2!=cl.end();it2++) {

			const region_info *ri = (*it2)->get_info();
			assert(ri);
			zindex_t z = ri->get_zindex();
			AM_DBG lib::logger::get_logger()->debug("surface_impl.redraw(0x%x %s) ->subregion 0x%x (z=%d)", (void *)this, m_name.c_str(), (void *)(*it2), z);
			if (z < last_z_index)
				lib::logger::get_logger()->debug("surface_impl.redraw(0x%x %s): Error: z-index %d for subregion 0x%x below previous z-index",  (void *)this, m_name.c_str(), z, (void *)(*it2));
			if (ri && !ri->is_subregion()) {
				AM_DBG lib::logger::get_logger()->debug("surface_impl.redraw(0x%x): subregion 0x%x is not a subregion", (void*)this, (void*)(*it2));
				//(*it2)->redraw(our_rect, window);
			}
			(*it2)->redraw(our_rect, window);
		}
	}

	// Then the children regions of this
	// XXXX Should go per z-order value
	last_z_index = -1;
	for(children_map_t::iterator it2=m_active_children.begin();it2!=m_active_children.end();it2++) {
		AM_DBG lib::logger::get_logger()->debug("surface_impl.redraw(0x%x %s) examining next z-order list", (void*)this, m_name.c_str());
		children_list_t& cl = (*it2).second;
		for(children_list_t::iterator it3=cl.begin();it3!=cl.end();it3++) {
			const region_info *ri = (*it3)->get_info();
			assert(ri);
			if(!ri || !ri->is_subregion()) {
				zindex_t z = ri->get_zindex();
				AM_DBG lib::logger::get_logger()->debug("surface_impl.redraw(0x%x %s) ->child region 0x%x (z=%d)", (void *)this, m_name.c_str(), (void *)(*it3), z);
				if (z < last_z_index)
					lib::logger::get_logger()->debug("surface_impl.redraw(0x%x %s): Error: z-index %d for child region 0x%x below previous z-index",  (void *)this, m_name.c_str(), z, (void *)(*it3));
				(*it3)->redraw(our_rect, window);
			}
		}
	}
	// Finally any highlighting needed
	if (m_highlighting && m_bg_renderer)
		m_bg_renderer->highlight(window);

	AM_DBG lib::logger::get_logger()->debug("surface_impl.redraw(0x%x %s) returning", (void*)this, m_name.c_str());
	m_children_cs.leave();
}

void
surface_impl::draw_background(const lib::rect &r, gui_window *window)
{
	// Now we should make sure we have a background renderer
	if (!m_bg_renderer) {
		AM_DBG lib::logger::get_logger()->debug("draw_background %s: no m_bg_renderer", m_name.c_str());
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("draw_background(0x%x %s): drawing background", (void*)this, m_name.c_str());
	m_bg_renderer->redraw(r, window);
}

bool
surface_impl::_is_active()
{
	if (m_renderers.begin() != m_renderers.end()) return true;
	
	for(children_map_t::iterator it1=m_active_children.begin();it1!=m_active_children.end();it1++) {
		children_list_t& cl = (*it1).second;
		for(children_list_t::iterator it2=cl.begin();it2!=cl.end();it2++) {
			if ((*it2)->_is_active()) return true;
		}
	}
	return false;
}

bool
surface_impl::user_event(const lib::point &where, int what)
{
	AM_DBG lib::logger::get_logger()->debug("surface_impl.user_event(0x%x '%s', (%d, %d))", (void *)this, m_name.c_str(), where.x, where.y);
	// Test that it is in our area
	if (!m_outer_bounds.contains(where)) {
		AM_DBG	if (what==0) lib::logger::get_logger()->debug("surface_impl.user_event: not in our bounds");
		return false;
	}
	// Convert to local coordinates
	point our_point = where;
	our_point -= m_outer_bounds.left_top();

	m_children_cs.enter();
	bool handled = false;
	// First check whether any of our subregions (which are on top of us) are interested.
	children_map_t::reverse_iterator it1;
	for(it1=m_active_children.rbegin();it1!=m_active_children.rend();it1++) {
		if (handled && what == user_event_click) break;
		children_list_t& cl = (*it1).second;
		children_list_t::reverse_iterator it2;
		for(it2=cl.rbegin();it2!=cl.rend();it2++) {
			if (handled && what == user_event_click) break;
			AM_DBG lib::logger::get_logger()->debug("surface_impl.user_event(0x%x) -> child 0x%x,z=%d", (void *)this, (void *)(*it2), (*it1).first);
			const region_info *ri = (*it2)->get_info();
			assert(ri);
			if(!ri || !ri->is_subregion()) {
				handled = (*it2)->user_event(our_point, what);
			}
		}
	}
	// Next check whether any of our node-based subregions (which are on top of us) are interested.
	for(it1=m_subregions.rbegin();it1!=m_subregions.rend();it1++) {
		if (handled && what == user_event_click) break;
		children_list_t& cl = (*it1).second;
		children_list_t::reverse_iterator it2;
		for(it2=cl.rbegin();it2!=cl.rend();it2++) {
			if (handled && what == user_event_click) break;
			AM_DBG lib::logger::get_logger()->debug("surface_impl.user_event(0x%x) -> node-child 0x%x,z=%d", (void *)this, (void *)(*it2), (*it1).first);
			handled = (*it2)->user_event(our_point, what);
		}
	}
	// Finally check our own renderers.
	std::list<gui_events*>::reverse_iterator ari;
	for (ari=m_renderers.rbegin(); ari!=m_renderers.rend(); ari++) {
		if (handled && what == user_event_click) break;
		AM_DBG lib::logger::get_logger()->debug("surface_impl.user_event(0x%x) -> active renderer 0x%x", (void *)this, (void *)(*ari));
		handled = (*ari)->user_event(our_point, what);
	}
	m_children_cs.leave();
	return handled;
}

void
surface_impl::need_redraw(const lib::rect &r)
{
	AM_DBG lib::logger::get_logger()->debug("surface_impl[0x%x].need_redraw(xywh=%d,%d,%d,%d)", (void*)this, r.left(), r.top(), r.width(), r.height());
	if (!m_parent)
		return;	  // Audio region or some such
	rect parent_rect = r & m_inner_bounds;
	m_parent->need_redraw(m_outer_bounds.outercoordinates(parent_rect));
}

void
surface_impl::need_redraw()
{
	need_redraw(m_inner_bounds);
}

void
surface_impl::need_events(bool want)
{
	if (!m_parent)
		return;	  // Audio region or some such
	// Not good enough: we only forward that we want events
	if (want) m_parent->need_events(want);
}

const lib::point &
surface_impl::get_global_topleft() const
{
	const_cast<surface_impl*>(this)->need_bounds();
	return m_window_topleft;
}

const lib::rect&
surface_impl::get_clipped_screen_rect() const
{
	const_cast<surface_impl*>(this)->need_bounds();
	return m_clipped_screen_bounds;
}

void
surface_impl::need_bounds()
{
	if (m_bounds_inited) return;
	AM_DBG lib::logger::get_logger()->debug("surface_impl::need_bounds(%s, 0x%x)", m_name.c_str(), (void*)this);
	if (m_info) m_outer_bounds = m_info->get_rect();
	AM_DBG lib::logger::get_logger()->debug("surface_impl::need_bounds: %d %d %d %d",
		m_outer_bounds.left(), m_outer_bounds.top(), m_outer_bounds.right(), m_outer_bounds.bottom());
	m_inner_bounds = m_outer_bounds.innercoordinates(m_outer_bounds);
	m_window_topleft = m_outer_bounds.left_top();
	if (m_parent) {
		m_parent->need_bounds();
		m_window_topleft += m_parent->get_global_topleft();
	}
	m_clipped_screen_bounds = m_inner_bounds;
	m_clipped_screen_bounds.translate(m_window_topleft);
	if (m_parent) {
		// need_bounds already called
		m_clipped_screen_bounds = m_clipped_screen_bounds & m_parent->m_clipped_screen_bounds;
	}
	m_bounds_inited = true;
}

void
surface_impl::clear_cache()
{
	AM_DBG lib::logger::get_logger()->debug("surface_impl::clear_cache(%s, 0x%x)", m_name.c_str(), (void*)this);
	m_bounds_inited = false;
	// Better be safe than sorry: clear caches for all child regions
	m_children_cs.enter();
	for(children_map_t::iterator it1=m_active_children.begin();it1!=m_active_children.end();it1++) {
		children_list_t& cl = (*it1).second;
		for(children_list_t::iterator it2=cl.begin();it2!=cl.end();it2++)
			(*it2)->clear_cache();
	}
	m_children_cs.leave();
	// XXXX note that this does *not* propagate the change to the
	// renderers yet: if they cache information they will have to
	// invalidate it on the next redraw.
}

lib::rect
surface_impl::get_crop_rect(const lib::size& srcsize) const
{
	if (m_info)
		return m_info->get_crop_rect(srcsize);
	return lib::rect(lib::point(0, 0), srcsize);
}

lib::rect
surface_impl::get_fit_rect(const lib::size& src_size, lib::rect* out_src_rect, const common::alignment *align) const
{
	lib::rect src_clip_rect(lib::point(0,0), src_size);
	return get_fit_rect(src_clip_rect, src_size, out_src_rect, align);
}

lib::rect
surface_impl::get_fit_rect(const lib::rect& src_clip_rect, const lib::size& src_real_size, lib::rect* out_src_rect, const common::alignment *align) const
{
	const lib::size src_size(src_clip_rect.width(), src_clip_rect.height());
	const_cast<surface_impl*>(this)->need_bounds();
	const int image_width = src_size.w;
	const int image_height = src_size.h;
	const int region_width = m_inner_bounds.width();
	const int region_height = m_inner_bounds.height();
	lib::size region_size = lib::size(region_width, region_height);
	lib::point xy_image(0, 0);
	lib::point xy_region(0, 0);
	if (align) {
		xy_image = align->get_image_fixpoint(src_size);
		xy_region = align->get_surface_fixpoint(region_size);
	}

	const int x_image_left = xy_image.x;
	const int x_image_right = image_width - xy_image.x;
	const int y_image_top = xy_image.y;
	const int y_image_bottom = image_height - xy_image.y;
	const int x_region_left = xy_region.x;
	const int x_region_right = region_width - xy_region.x;
	const int y_region_top = xy_region.y;
	const int y_region_bottom = region_height - xy_region.y;

	AM_DBG lib::logger::get_logger()->debug("get_fit_rect: image size=(%d, %d)", image_width, image_height);
	AM_DBG lib::logger::get_logger()->debug("get_fit_rect: region size=(%d, %d)", region_width, region_height);
	AM_DBG lib::logger::get_logger()->debug("get_fit_rect: image fixpoint=(%d, %d)", xy_image.x, xy_image.y);
	AM_DBG lib::logger::get_logger()->debug("get_fit_rect: region fixpoint=(%d, %d)", xy_region.x, xy_region.y);
	AM_DBG lib::logger::get_logger()->debug("get_fit_rect: image delta fixpoint to ltrb=(%d, %d, %d, %d)", x_image_left, y_image_top, x_image_right, y_image_bottom);
	AM_DBG lib::logger::get_logger()->debug("get_fit_rect: region delta fixpoint to ltrb=(%d, %d, %d, %d)", x_region_left, y_region_top, x_region_right, y_region_bottom);

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

	fit_t fit = (m_info == NULL?fit_default : m_info->get_fit());
	// This is a bit of a hack: if no fit value is specified we pick it up
	// from our parent. This is needed for subregions (nodes).
	if (fit == fit_default && m_parent && m_parent->m_info)
		fit = m_parent->m_info->get_fit();
	switch (fit) {
	case fit_fill:
		// Fill the area with the image, ignore aspect ration
		// XXX I don't think this is correct. Or is it?
		scale_horizontal = scale_min_horizontal;
		scale_vertical = scale_min_vertical;
		break;
	case fit_scroll:		// XXXX incorrect
	case fit_default:
	case fit_hidden:
		scale_horizontal = scale_vertical = 1.0;
		break;
	case fit_meet:
		// Scale to make smallest edge fit (showing some background color)
		scale_horizontal = scale_vertical = std::min(scale_min_horizontal, scale_min_vertical);
		break;
	case fit_meetbest:
		// Scale to make smallest edge fit (showing some background color)
		scale_vertical = std::min(scale_min_horizontal, scale_min_vertical);
		if (scale_vertical > 1.0) scale_vertical = 1.0;
		scale_horizontal = scale_vertical;
		break;
	case fit_slice:
		// Scale to make largest edge fit (not showing the full source image)
		scale_horizontal = scale_vertical = std::max(scale_max_horizontal, scale_max_vertical);
		break;
	}
	AM_DBG lib::logger::get_logger()->debug("get_fit_rect: scale_hor=%f, scale_vert=%f", scale_horizontal, scale_vertical);
	if (scale_horizontal == 0 || scale_vertical == 0) {
		*out_src_rect = lib::rect(point(0,0), size(0,0));
		return lib::rect(point(0,0), size(0,0));
	}
	// Convert the image fixpoint to scaled coordinates
	int x_image_scaled = (int)((xy_image.x * scale_horizontal) + 0.5);
	int y_image_scaled = (int)((xy_image.y * scale_vertical) + 0.5);
	AM_DBG lib::logger::get_logger()->debug("get_fit_rect: scaled image fixpoint=(%d, %d)", x_image_scaled, y_image_scaled);
	// Find out where image (0, 0) would end up, and similarly for other
	// corners. At this point we still allow negative values or values > max
	int x_region_for_image_left = xy_region.x - x_image_scaled;
	int x_region_for_image_right = x_region_for_image_left + (int)((image_width * scale_horizontal) + 0.5);
	int y_region_for_image_top = xy_region.y - y_image_scaled;
	int y_region_for_image_bottom = y_region_for_image_top + (int)((image_height * scale_vertical) + 0.5);
	int x_image_for_region_left = src_clip_rect.left();
	int x_image_for_region_right = src_clip_rect.right();
	int y_image_for_region_top = src_clip_rect.top();
	int y_image_for_region_bottom = src_clip_rect.bottom();
	// Now we need to clamp the image values: the panZoom coordinates could be outside the
	// real image coordinate space.
	if (x_image_for_region_left < 0) {
		x_region_for_image_left = (int)((-x_image_for_region_left * scale_horizontal) + 0.5);
		x_image_for_region_left = 0;
	}
	if (x_image_for_region_right > (int)src_real_size.w) {
		int overshoot = x_image_for_region_right - src_real_size.w;
		x_image_for_region_right = src_real_size.w;
		x_region_for_image_right = x_region_for_image_right - (int)((overshoot * scale_horizontal) + 0.5);
	}
	if (y_image_for_region_top < 0) {
		y_region_for_image_top = (int)((-y_image_for_region_top * scale_horizontal) + 0.5);
		y_image_for_region_top = 0;
	}
	if (y_image_for_region_bottom > (int)src_real_size.h) {
		int overshoot = y_image_for_region_bottom - src_real_size.h;
		y_image_for_region_bottom = src_real_size.h;
		y_region_for_image_bottom = y_region_for_image_bottom - (int)((overshoot * scale_vertical) + 0.5);
	}
	AM_DBG lib::logger::get_logger()->debug("get_fit_rect: full image would	 have region lrtb=(%d, %d, %d, %d)",
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
	AM_DBG lib::logger::get_logger()->debug("get_fit_rect: image selection ltrb=(%d, %d, %d, %d)",
		x_image_for_region_left, y_image_for_region_top, x_image_for_region_right, y_image_for_region_bottom);
	AM_DBG lib::logger::get_logger()->debug("get_fit_rect: region selection lrtb=(%d, %d, %d, %d)",
		x_region_for_image_left, y_region_for_image_top, x_region_for_image_right, y_region_for_image_bottom);
	*out_src_rect = lib::rect(
		point(x_image_for_region_left, y_image_for_region_top),
		size(x_image_for_region_right-x_image_for_region_left, y_image_for_region_bottom-y_image_for_region_top));
	return lib::rect(
		point(x_region_for_image_left, y_region_for_image_top),
		size(x_region_for_image_right-x_region_for_image_left, y_region_for_image_bottom-y_region_for_image_top));
}

bool
surface_impl::is_tiled() const
{
	common::tiling t = m_info->get_tiling();

	return (t == common::tiling_horizontal || t == common::tiling_vertical || t == common::tiling_both || t == common::tiling_default);
}

tile_positions
surface_impl::get_tiles(lib::size image_size, lib::rect surface_rect) const
{
	assert(is_tiled());

	tile_positions rv;

	int x, y;
	int width = image_size.w;
	int height = image_size.h;
	int max_x = surface_rect.left()+width;
	int max_y = surface_rect.top()+height;
	common::tiling t = m_info->get_tiling();
	if (t == common::tiling_horizontal || t == common::tiling_both || t == common::tiling_default)
		max_x = surface_rect.right();
	if (t == common::tiling_vertical || t == common::tiling_both || t == common::tiling_default)
		max_y = surface_rect.bottom();

	for (x=surface_rect.left(); x < max_x; x += width) {
		for (y=surface_rect.top(); y < max_y; y += height) {
			int w = std::min<int>(width, max_x-x);
			int h = std::min<int>(height, max_y-y);
			rect srcrect(point(0, 0), size(w, h));
			rect dstrect(point(x, y), size(w, h));
			rv.push_back(common::tile_position(srcrect, dstrect));
		}
	}
	return rv;
}

void
surface_impl::transition_done(lib::rect area)
{
	if (!m_parent)
		return;	  // Audio region or some such
	area &= m_inner_bounds;
	m_parent->transition_done(m_outer_bounds.outercoordinates(area));
}

void
surface_impl::transition_freeze_end(lib::rect r)
{
	AM_DBG lib::logger::get_logger()->debug("surface_impl.transition_freeze_end(0x%x %s, ltrb=(%d, %d, %d, %d))", (void *)this, m_name.c_str(), r.left(), r.top(), r.right(), r.bottom());
	r &= m_outer_bounds;
	r = m_outer_bounds.innercoordinates(r);
	if (r.empty()) {
	AM_DBG lib::logger::get_logger()->debug("surface_impl.transition_freeze_end(0x%x %s) returning: no overlap", (void *)this, m_name.c_str());
		return;
	}

	// Signal the active renderers
	// For the win32 arrangement we should have at most one active
	m_children_cs.enter();
	assert(m_renderers.size()<=1);
	std::list<gui_events*>::iterator ar;
	for (ar=m_renderers.begin(); ar!=m_renderers.end(); ar++) {
		AM_DBG lib::logger::get_logger()->debug("surface_impl.transition_freeze_end(0x%x %s) ->renderer 0x%x", (void *)this, m_name.c_str(), (void *)(*ar));
		(*ar)->transition_freeze_end(r);
	}

	// Finally the children regions of this
	for(children_map_t::iterator it2=m_active_children.begin();it2!=m_active_children.end();it2++) {
		AM_DBG lib::logger::get_logger()->debug("surface_impl.transition_freeze_end(0x%x %s) examining next z-order list", (void*)this, m_name.c_str());
		children_list_t& cl = (*it2).second;
		for(children_list_t::iterator it3=cl.begin();it3!=cl.end();it3++) {
			AM_DBG lib::logger::get_logger()->debug("surface_impl.transition_freeze_end(0x%x %s) -> child 0x%x", (void *)this, m_name.c_str(), (void *)(*it3));
			(*it3)->transition_freeze_end(r);
		}
	}
	AM_DBG lib::logger::get_logger()->debug("surface_impl.transition_freeze_end(0x%x %s) returning", (void*)this, m_name.c_str());
	m_children_cs.leave();
}

void
surface_impl::add_subregion(zindex_t z, surface_impl *rgn)
{
	m_children_cs.enter();
	bool was_inactive = !_is_active();
	m_subregions[z].push_back(rgn);
	m_children_cs.leave();
	if (was_inactive) background_render_changed();
}

void
surface_impl::del_subregion(zindex_t z, surface_impl *rgn)
{
	m_children_cs.enter();
	m_subregions[z].remove(rgn);
	bool now_inactive = !_is_active();
	m_children_cs.leave();
	if (now_inactive) background_render_changed();
}

void
surface_impl::background_render_changed()
{
	// If we are opaque and have showBackground=whenActive we schedule a redraw
	const region_info *info = get_info();
	if (info && !info->get_transparent() && !info->get_showbackground()) {
		need_redraw();
	}
	// We also forward to our parent
	if (m_parent) {
		m_parent->background_render_changed();
	}
}

renderer_private_data*
surface_impl::get_renderer_private_data(renderer_private_id idd) {
	if (m_info && m_info->is_subregion()) {
		assert(m_parent);
		return m_parent->get_renderer_private_data(idd);
	}
	if (idd == m_renderer_id)
		return m_renderer_data;
	return NULL;
}

void
surface_impl::set_renderer_private_data(renderer_private_id idd, renderer_private_data* data) {
	if (m_info && m_info->is_subregion()) {
		assert(m_parent);
		m_parent->set_renderer_private_data(idd, data);
	}
	// Release the old data
	if (m_renderer_data)
		m_renderer_data->release();
	m_renderer_data = data;
	m_renderer_data->add_ref();
	m_renderer_id = idd;
}

void
surface_impl::highlight(bool onoff)
{
	m_highlighting = onoff;
	need_redraw(m_inner_bounds);
}

void
surface_impl::set_forced_redraw()
{
	m_forced_redraw = true;
	if (m_parent) m_parent->set_forced_redraw();
}

// toplevel_surface_impl

toplevel_surface_impl::toplevel_surface_impl(const region_info *info, lib::size bounds, bgrenderer *bgrenderer, window_factory *wf)
:
	surface_impl(info?info->get_name():"topLayout", NULL, rect(point(0, 0), bounds), info, bgrenderer)
{
	m_gui_window = wf->new_window(m_name, bounds, this);
	AM_DBG lib::logger::get_logger()->debug("toplevel_surface_impl(0x%x, \"%s\"): window=0x%x", (void *)this, m_name.c_str(), (void *)m_gui_window);
}

toplevel_surface_impl::~toplevel_surface_impl()
{
	AM_DBG lib::logger::get_logger()->debug("~toplevel_surface_impl(0x%x)", (void*)this);
	if (m_gui_window)
		delete m_gui_window;
	m_gui_window = NULL;
}

void
toplevel_surface_impl::need_redraw(const lib::rect &r)
{
	if (m_gui_window)
		m_gui_window->need_redraw(r);
	else {
		lib::logger::get_logger()->debug("toplevel_surface_impl::need_redraw: m_gui_window == NULL");
		lib::logger::get_logger()->warn(gettext("Programmer error encountered, will attempt to continue"));
	}
}

void
toplevel_surface_impl::need_events(bool want)
{
	if (m_gui_window)
		m_gui_window->need_events(want);
	else {
		lib::logger::get_logger()->trace("toplevel_surface_impl::need_events: m_gui_window == NULL");
		lib::logger::get_logger()->warn(gettext("Programmer error encountered, will attempt to continue"));
	}
}


