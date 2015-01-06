/*
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 *
 * Copyright (C) 2003-2015 Stichting CWI, 
 * Science Park 123, 1098 XG Amsterdam, The Netherlands.
 *
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef AMBULANT_COMMON_REGION_H
#define AMBULANT_COMMON_REGION_H

#include "ambulant/config/config.h"

#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/common/layout.h"
#include "ambulant/common/region_info.h"

namespace ambulant {

namespace common {

using namespace ambulant::lib;

class surface_impl : public surface_template, public surface, public gui_events {
  // The only constructor is protected:
  protected:
	surface_impl(const std::string &name, surface_impl *parent, rect bounds,
		const region_info *info, bgrenderer *bgrenderer);
  public:
	virtual ~surface_impl();

	// The surface_template interface:
	common::surface_template *new_subsurface(const region_info *info, bgrenderer *bgrenderer);
	surface *activate();
	void animated();

	// The surface interface:
	void show(gui_events *cur);
	void renderer_done(gui_events *renderer);
	virtual void need_redraw(const rect &r);
	void need_redraw();
	virtual void need_events(bool want);
	const rect& get_rect() const { return m_inner_bounds; }
	const rect& get_clipped_screen_rect() const;
	virtual const point &get_global_topleft() const;
	rect get_fit_rect(const size& src_size, rect* out_src_rect, const common::alignment *align) const;
	rect get_fit_rect(const rect& src_clip_rect, const size& src_size, rect* out_src_rect, const common::alignment *align) const;
	rect get_crop_rect(const size& src_size) const;
	bool is_tiled() const;
	tile_positions get_tiles(lib::size image_size, lib::rect surface_rect) const;
	const region_info *get_info() const { return m_info; }
	surface *get_top_surface() { return m_parent->get_top_surface(); }
	gui_window *get_gui_window() { return m_parent->get_gui_window(); }

	void transition_done() { transition_done(m_inner_bounds); }

	void keep_as_background();
	renderer_private_data* get_renderer_private_data(renderer_private_id idd);
	void set_renderer_private_data(renderer_private_id idd, renderer_private_data* data);
	void highlight(bool on);
	// The gui_events interface:
	void redraw(const rect &dirty, gui_window *window);
	bool user_event(const point &where, int what = 0);

	// Win32 code needs this, but I don't like it:
	const surface_impl *get_parent() const { return m_parent; }

  private:
	void clear_cache();					// invalidate cached sizes (after animation)
	void need_bounds();					// recompute cached sizes
	rect get_fit_rect_noalign(const size& src_real_size, rect* out_src_rect) const;
	void draw_background(const rect &r, gui_window *window);
	bool _is_active();                  // Return true if region is active
	void set_forced_redraw();			// Make sure the next redraw is propagated even if empty.

  protected:
	virtual void transition_done(lib::rect area);
	void transition_freeze_end(lib::rect area);
	void background_render_changed();   // Called when drawing the background may have changed

	std::string m_name;					// for debugging

	bool m_bounds_inited;				// True if bounds and topleft initialized
	bool m_highlighting;
	rect m_inner_bounds;	// region rectangle (0, 0) based
	rect m_outer_bounds;	// region rectangle in parent coordinate space
	rect m_clipped_screen_bounds;	// Clipped region rectangle in screen coordinates
	point m_window_topleft;				// region top-left in window coordinate space

	surface_impl *m_parent;			// parent region

	std::list<gui_events *> m_renderers; // active regions currently responsible for redraws
	lib::critical_section m_children_cs; // Protects m_renderers, m_active_children, m_subregions

	typedef std::list<surface_impl*> children_list_t;
	typedef std::map<zindex_t, children_list_t> children_map_t;
	children_map_t m_active_children;	// all child regions
	children_map_t m_subregions;		// all active children that are subregions
	void add_subregion(zindex_t z, surface_impl *rgn);
	void del_subregion(zindex_t z, surface_impl *rgn);

	const region_info *m_info;			// Information such as z-order, etc.
	bgrenderer *m_bg_renderer;			// Background renderer
	renderer_private_data *m_renderer_data;	// per-renderer private data pointer
	renderer_private_id m_renderer_id;		// owner of m_renderer_data
	bool m_forced_redraw;
};

class toplevel_surface_impl : public surface_impl {
  public:
	toplevel_surface_impl(const region_info *info, size bounds, bgrenderer *bgrenderer, window_factory *wf);
	~toplevel_surface_impl();

	void need_redraw(const rect &r);
	void need_events(bool want);
	const point &get_global_topleft() const { static point p = point(0, 0); return p; }
	surface *get_top_surface() { return this; }
	gui_window *get_gui_window() { return m_gui_window; }
  protected:
	void transition_done(lib::rect area) { transition_freeze_end(area); }
    
  private:
	gui_window *m_gui_window;
};

class smil_surface_factory : public surface_factory {
  public:
	surface_template *new_topsurface(const region_info *info, bgrenderer *bgrend, window_factory *wf);
};

} // namespace common

} // namespace ambulant

#endif // AMBULANT_COMMON_REGION_H
