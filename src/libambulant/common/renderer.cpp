/* 
 * @$Id$ 
 */

/*
 * The contents of this file are subject to the Public
 * License XXX. 
 *
 */

#include "ambulant/lib/logger.h"
#include "ambulant/common/renderer.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

typedef lib::no_arg_callback<lib::active_renderer> readdone_callback;

lib::active_renderer::active_renderer(event_processor *const evp,
	net::passive_datasource *src,
	passive_region *const dest,
	const node *node)
:	active_basic_renderer(evp, node),
	m_src(src?src->activate():NULL),
	m_dest(dest->activate(evp, node)),
	m_readdone(NULL),
	m_playdone(NULL)
{
	m_readdone = new readdone_callback(this, &lib::active_renderer::readdone);
}

void
lib::active_renderer::start(lib::event *playdone)
{
	if (!m_node) abort();
	m_playdone = playdone;
	std::ostringstream os;
	os << *m_node;
	AM_DBG lib::logger::get_logger()->trace("active_renderer.start(0x%x, %s, playdone=0x%x)", (void *)this, os.str().c_str(), (void *)playdone);
	m_dest->show(this);
	if (m_src) {
		m_src->start(m_event_processor, m_readdone);
	} else {
		lib::logger::get_logger()->error("active_renderer.start: no datasource");
		if (m_playdone)
			m_event_processor->add_event(m_playdone, 0, event_processor::low);
	}
}

void
lib::active_renderer::readdone()
{
	AM_DBG lib::logger::get_logger()->trace("active_renderer.readdone(0x%x, size=%d)", (void *)this, m_src->size());
	m_dest->need_redraw();
	if (m_playdone)
		m_event_processor->add_event(m_playdone, 0, event_processor::low);
}

void
lib::active_renderer::stop()
{
	// XXXX Need to handle case that no data (or not all data) has come in yet
	m_dest->done();
	AM_DBG lib::logger::get_logger()->trace("active_renderer.stop(0x%x)", (void *)this);
}

lib::active_final_renderer::~active_final_renderer()
{
	if (m_data) free(m_data);
}

void
lib::active_final_renderer::readdone()
{
	AM_DBG lib::logger::get_logger()->trace("active_final_renderer.readdone(0x%x, size=%d)", (void *)this, m_src->size());
	m_data_size = m_src->size();
	if ((m_data = malloc(m_data_size)) == NULL) {
		lib::logger::get_logger()->fatal("active_final_renderer.readdone: cannot allocate %d bytes", m_data_size);
		abort();
	}
	m_src->read((char *)m_data, m_data_size);
	m_dest->need_redraw();
	if (m_playdone)
		m_event_processor->add_event(m_playdone, 0, event_processor::low);
}


