/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/renderer.h"
#include "ambulant/lib/logger.h"

using namespace ambulant;


void
lib::active_renderer::start(lib::event *playdone)
{
	std::ostringstream os;
	os << *m_node;
	lib::logger::get_logger()->trace("active_renderer.start(0x%x, %s, playdone=0x%x)", (void *)this, os.str().c_str(), (void *)playdone);
	m_dest->show();
	if (playdone)
		m_event_processor->add_event(playdone, 0, event_processor::low);
}

void
lib::active_renderer::stop()
{
	m_dest->done();
	lib::logger::get_logger()->trace("active_renderer.stop(0x%x)", (void *)this);
}
