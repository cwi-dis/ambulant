
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

#include "ambulant/lib/node.h"
#include "ambulant/lib/callback.h"
#include "ambulant/lib/event_processor.h"

namespace ambulant {

namespace lib {

// Forward
class active_region;

class passive_region {
  public:
	friend class active_region;
	
	passive_region() 
	:	m_name("unnamed") {}
	passive_region(char *name)
	:	m_name(name) {}
	~passive_region() {}
	
	active_region *activate(event_processor *const evp, const node *node);
  private:
  	char *m_name; // for debugging
};

class active_region {
  public:
	active_region(event_processor *const evp,
		passive_region *const source,
		const node *node)
	:	m_event_processor(evp),
		m_source(source),
		m_node(node) {}
	~active_region() {}
	
	void start(event *playdone);
	void stop();
	

  private:
  	event_processor *const m_event_processor;
	passive_region *const m_source;
	const node *m_node;
};

} // namespace lib
 
} // namespace ambulant

#endif // AMBULANT_LIB_REGION_H
