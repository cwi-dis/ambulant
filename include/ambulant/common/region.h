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

#ifndef AMBULANT_LIB_REGION_H
#define AMBULANT_LIB_REGION_H

#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/event_processor.h"
#include "ambulant/lib/layout.h"

namespace ambulant {

namespace lib {

// Forward
class active_region;
class active_renderer;
class passive_window;

// NOTE: the "bounds" rectangles are currently all with respect
// to the parent, and in a coordinate system where (0,0) is the
// topleft point in the rectangle.
class passive_region : public abstract_rendering_source {
  public:
	friend class active_region;
	
	passive_region() 
	:	m_name("unnamed"),
		m_inner_bounds(screen_rect<int>()),
		m_outer_bounds(screen_rect<int>()),
		m_window_topleft(point(0, 0)),
		m_parent(NULL),
		m_cur_active_region(NULL) {}
	passive_region(const std::string &name)
	:	m_name(name),
		m_inner_bounds(screen_rect<int>()),
		m_outer_bounds(screen_rect<int>()),
		m_window_topleft(point(0, 0)),
		m_parent(NULL),
		m_cur_active_region(NULL) {}
	virtual ~passive_region() {}
	
	virtual void show(active_region *cur);
	virtual void redraw(const screen_rect<int> &dirty, abstract_window *window);

	virtual passive_region *subregion(const std::string &name, screen_rect<int> bounds);
	active_region *activate(const node *node);
	
	const screen_rect<int>& get_rect() const { return m_inner_bounds; }
	const screen_rect<int>& get_rect_outer() const { return m_outer_bounds; }
	const point &get_global_topleft() const { return m_window_topleft; }
	const passive_region* get_parent() const { return m_parent; }
	
  protected:
	passive_region(const std::string &name, passive_region *parent, screen_rect<int> bounds,
		point window_topleft)
	:	m_name(name),
		m_inner_bounds(bounds.innercoordinates(bounds)),
		m_outer_bounds(bounds),
		m_window_topleft(window_topleft),
		m_parent(parent),
		m_cur_active_region(NULL) {}
	virtual void need_redraw(const screen_rect<int> &r);

  	std::string m_name;					// for debugging
  	screen_rect<int> m_inner_bounds;	// region rectangle (0, 0) based
  	screen_rect<int> m_outer_bounds;	// region rectangle in parent coordinate space
	point m_window_topleft;				// region top-left in window coordinate space
  	passive_region *m_parent;			// parent region
  	active_region *m_cur_active_region; // active region currently responsible for redraws
  	std::vector<passive_region *>m_children;	// all subregions
};

class passive_root_layout : public passive_region {
  public:
	passive_root_layout(const std::string &name, size bounds, window_factory *wf);
	~passive_root_layout();
	void need_redraw(const screen_rect<int> &r);
  private:
	abstract_window *m_gui_window;
};


#ifdef __OBJC__
// This is a workaround for a problem when using gcc 3.3 to compile
// ObjC++
;
#endif

class active_region : public abstract_rendering_surface, public abstract_rendering_source {
  public:
	active_region(passive_region *const source,
		const node *node)
	:	m_source(source),
		m_node(node),
		m_renderer(NULL) {}
	virtual ~active_region() {}
	
	virtual void show(abstract_rendering_source *renderer);
	virtual void redraw(const screen_rect<int> &dirty, abstract_window *window);
	virtual void need_redraw(const screen_rect<int> &r);
	virtual void need_redraw();
	virtual void renderer_done();	
	const screen_rect<int>& get_rect() const { return m_source->get_rect(); }
	const screen_rect<int>& get_rect_outer() const { return m_source->get_rect_outer(); }
	const point &get_global_topleft() const { return m_source->get_global_topleft(); }
	const passive_region* get_parent() const { return m_source->get_parent(); }
	
  protected:
	passive_region *const m_source;
	const node *m_node;
	abstract_rendering_source *m_renderer;
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_REGION_H
