/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/region.h"
#include "ambulant/lib/logger.h"

using namespace ambulant;

lib::active_region *
lib::passive_region::activate(event_processor *const evp, const node *node)
{
	return new lib::active_region(evp, this, node);
}

void
lib::active_region::start(event *playdone)
{
	lib::logger::get_logger()->trace("active_region.start(0x%x, \"%s\", playdone=0x%x)", (void *)this, m_source->m_name, (void *)playdone);
	if (playdone)
		m_event_processor->add_event(playdone, 0, event_processor::low);
}

void
lib::active_region::stop()
{
	lib::logger::get_logger()->trace("active_region.stop(0x%x, \"%s\")", (void *)this, m_source->m_name);
}
