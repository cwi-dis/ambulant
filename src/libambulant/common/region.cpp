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
lib::active_region::show()
{
	lib::logger::get_logger()->trace("active_region.show(0x%x, \"%s\")", (void *)this, m_source->m_name);
}

void
lib::active_region::done()
{
	lib::logger::get_logger()->trace("active_region.done(0x%x, \"%s\")", (void *)this, m_source->m_name);
}
