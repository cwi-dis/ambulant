
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

class passive_region {
  public:
	friend class active_region;
	
	passive_region() 
	:	m_name("unnamed"),
		m_bounds(screen_rect<int>(0, 0, 0, 0)) {}
	passive_region(char *name)
	:	m_name(name),
		m_bounds(screen_rect<int>(0, 0, 0, 0)) {}
	passive_region(char *name, screen_rect<int> bounds)
	:	m_name(name),
		m_bounds(bounds) {}
	~passive_region() {}
	
	active_region *activate(event_processor *const evp, const node *node);
  private:
  	char *m_name; // for debugging
  	screen_rect<int> m_bounds;
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
	~active_region() {}
	
	virtual void show(active_renderer *renderer);
	virtual void redraw(const screen_rect<int> &r);
	virtual void need_redraw(const screen_rect<int> &r);
	virtual void need_redraw();
	virtual void done();	

  private:
  	event_processor *const m_event_processor;
	passive_region *const m_source;
	const node *m_node;
	active_renderer *m_renderer;
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_REGION_H
