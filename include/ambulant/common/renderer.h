
/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

 
#ifndef AMBULANT_LIB_RENDERER_H
#define AMBULANT_LIB_RENDERER_H

#include "ambulant/lib/node.h"
#include "ambulant/lib/region.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/event_processor.h"

namespace ambulant {

namespace lib {

// Forward

class active_renderer {
  public:
	active_renderer(event_processor *const evp,
		passive_region *const dest,
		const node *node)
	:	m_event_processor(evp),
		m_dest(dest->activate(evp, node)),
		m_node(node) {}
	~active_renderer() {}
	
	void start(event *playdone);
	void stop();
	

  private:
  	event_processor *const m_event_processor;
	active_region *const m_dest;
	const node *m_node;
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_RENDERER_H
