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

using namespace lib;

// Forward
class active_region;
class active_renderer;
class passive_window;

// NOTE: the "bounds" rectangles are currently all with respect
// to the parent, and in a coordinate system where (0,0) is the
// topleft point in the rectangle.
class passive_region : public surface_template, public renderer {
	friend class active_region;

  protected:
	passive_region(const std::string &name, passive_region *parent, screen_rect<int> bounds,
		const region_info *info, renderer *bgrenderer);
  public:
	virtual ~passive_region();
	
	virtual void show(active_region *cur);
	virtual void active_region_done(active_region *cur);
	virtual void redraw(const screen_rect<int> &dirty, abstract_window *window);
	virtual void user_event(const point &where);
	virtual void mouse_region_changed();
        
	virtual common::surface_template *new_subsurface(const region_info *info, renderer *bgrenderer);
	surface *activate();
	
	const screen_rect<int>& get_rect() const { return m_inner_bounds; }
	const screen_rect<int>& get_rect_outer() const { return m_outer_bounds; }
	virtual const point &get_global_topleft() const;
	const passive_region* get_parent() const { return m_parent; }
	const gui_region& get_mouse_region() const { return *m_mouse_region; }
	const region_info *get_info() const { return m_info; }	
		
	screen_rect<int> get_fit_rect(const size& src_size, rect* out_src_rect) const;
	screen_rect<int> get_fit_rect(const size& src_size, const alignment *align, rect* out_src_rect) const;
  protected:
	virtual void need_redraw(const screen_rect<int> &r);
	virtual void need_events(gui_region *rgn);
	virtual void clear_cache();
  private:
	// This is part of the surface interface that we don't export
	void show(renderer *rend) {abort();};
	void renderer_done() {abort();};
	void need_redraw() {abort();};
	void need_events(bool want) {abort();};
	// And some renderer interface we don't support:
	void set_surface(surface *dest) {abort(); }
	surface *get_surface() {abort(); }

	void need_bounds();
	void draw_background(const screen_rect<int> &r, abstract_window *window);
  protected:
  	std::string m_name;					// for debugging
	const char *m_name_str;					// ditto
	bool m_bounds_inited;					// True if bounds and topleft initialized
  	screen_rect<int> m_inner_bounds;	// region rectangle (0, 0) based XXXX do lazy
  	screen_rect<int> m_outer_bounds;	// region rectangle in parent coordinate space XXXX do lazy
	point m_window_topleft;				// region top-left in window coordinate space XXXX do lazy
  	passive_region *m_parent;			// parent region
  	active_region *m_cur_active_region; // active region currently responsible for redraws
  	active_region *m_bg_active_region; // active region responsible for background redraws
	active_region *m_old_active_region; // previous active region (for transitions)
  	std::multimap<zindex_t,passive_region*>m_active_children;	// all subregions
	gui_region *m_mouse_region;   // The area in which we want mouse clicks
	const region_info *m_info;	// Information such as z-order, etc.
	renderer *m_bg_renderer;  // Background renderer
};

class passive_root_layout : public passive_region {
  public:
	passive_root_layout(const region_info *info, size bounds, renderer *bgrenderer, window_factory *wf);
	~passive_root_layout();
	void need_redraw(const screen_rect<int> &r);
	void mouse_region_changed();
	const point &get_global_topleft() const { static point p = point(0, 0); return p; }
  private:
	abstract_window *m_gui_window;
};


#ifdef __OBJC__
// This is a workaround for a problem when using gcc 3.3 to compile
// ObjC++
;
#endif

class active_region : public surface, public renderer {
  public:
	active_region(passive_region *const source)
	:	m_source(source),
		m_renderer(NULL),
		m_mouse_region(NULL)
        {
			if (source->m_mouse_region) {
				m_mouse_region = source->m_mouse_region->clone();
				m_mouse_region->clear();
			}
        }
	virtual ~active_region();
	
	virtual void show(renderer *rend);
	virtual void redraw(const screen_rect<int> &dirty, abstract_window *window);
	virtual void user_event(const point &where);
	virtual void need_redraw(const screen_rect<int> &r);
	virtual void need_redraw();
	virtual void need_events(bool want);

	virtual void renderer_done();	
	const screen_rect<int>& get_rect() const { return m_source->get_rect(); }
	const screen_rect<int>& get_rect_outer() const { return m_source->get_rect_outer(); }
	const point &get_global_topleft() const { return m_source->get_global_topleft(); }
	const passive_region* get_parent() const { return m_source->get_parent(); }
	const gui_region& get_mouse_region() const { return *m_mouse_region; }
	const region_info *get_info() const { return m_source->m_info; }	
	screen_rect<int> get_fit_rect(const size& src_size, rect* out_src_rect) const
	{
		return m_source->get_fit_rect(src_size, out_src_rect);
	}
	screen_rect<int> get_fit_rect(const size& src_size, const alignment *align, 
		rect* out_src_rect) const
	{
		return m_source->get_fit_rect(src_size, align, out_src_rect);
	}
	
	// And some renderer interface we don't support:
	void set_surface(surface *dest) {abort(); }
	surface *get_surface() {abort(); }

  protected:
	passive_region *const m_source;
	renderer *m_renderer;
	gui_region *m_mouse_region;   // The area in which we want mouse clicks
};

class smil_surface_factory : public surface_factory {
  public:
	surface_template *new_topsurface(const region_info *info, renderer *bgrend, window_factory *wf);
};

} // namespace common
 
} // namespace ambulant

#endif // AMBULANT_COMMON_REGION_H
