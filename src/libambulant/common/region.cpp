/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/region.h"

namespace ambulant {

namespace lib {


active_region *
passive_region::activate(event_processor *const evp, node *node)
{
	return new active_region(evp, this, node);
}

void
active_region::start(event *playdone)
{
	std::cout << "active_region.start(" << m_source->m_name << ", playdone=" << (void *)playdone << ")" << std::endl;
	if (playdone)
		m_event_processor->add_event(playdone, 0, event_processor::low);
}

void
active_region::stop()
{
	std::cout << "active_region.stop(" << m_source->m_name << ")" << std::endl;
}

} // namespace lib
 
} // namespace ambulant
