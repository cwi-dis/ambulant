
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_REGION_H
#define AMBULANT_LIB_REGION_H

#include "ambulant/lib/gtypes.h"
#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/event_processor.h"

namespace ambulant {

namespace lib {

// Forward
class active_region;
class active_renderer;
class passive_window;

// NOTE: the "bounds" rectangles are currently all with respect
// to the parent, and in a coordinate system where (0,0) is the
// topleft point in the rectangle.
class passive_region {
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
	virtual void redraw(const screen_rect<int> &dirty, passive_window *window);

	virtual passive_region *subregion(const std::string &name, screen_rect<int> bounds);
	active_region *activate(event_processor *const evp, const node *node);
	
	const screen_rect<int>& get_rect() const { return m_inner_bounds; }
	const screen_rect<int>& get_rect_outer() const { return m_outer_bounds; }
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

#ifdef __OBJC__
// This is a workaround for a problem when using gcc 3.3 to compile
// ObjC++
;
#endif

class passive_window : public passive_region {
  public:
  	passive_window(const std::string &name, size bounds)
  	:	passive_region(name, NULL, screen_rect<int>(point(0, 0), size(bounds.w, bounds.h)),
		point(0, 0)) {}
  	virtual ~passive_window() {}
  	
	virtual void need_redraw(const screen_rect<int> &r);
};

#ifdef __OBJC__
// This is a workaround for a problem when using gcc 3.3 to compile
// ObjC++
;
#endif

class window_factory {
  public:
	virtual ~window_factory() {}
	virtual passive_window *new_window(const std::string &name, size bounds) = 0;
};

class active_region {
  public:
	active_region(event_processor *const evp,
		passive_region *const source,
		const node *node)
	:	m_event_processor(evp),
		m_source(source),
		m_node(node),
		m_renderer(NULL) {}
	virtual ~active_region() {}
	
	virtual void show(active_renderer *renderer);
	virtual void redraw(const screen_rect<int> &dirty, passive_window *window, const point &window_topleft);
	virtual void need_redraw(const screen_rect<int> &r);
	virtual void need_redraw();
	virtual void done();	
	const screen_rect<int>& get_rect() const { return m_source->get_rect(); }
	const screen_rect<int>& get_rect_outer() const { return m_source->get_rect_outer(); }
	const passive_region* get_parent() const { return m_source->get_parent(); }
	
  protected:
  	event_processor *const m_event_processor;
	passive_region *const m_source;
	const node *m_node;
	active_renderer *m_renderer;
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_REGION_H
