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

typedef lib::callback<lib::active_renderer,lib::detail::readdone_callback_arg> readdone_callback;

lib::active_renderer::active_renderer(event_processor *const evp,
	net::passive_datasource *src,
	passive_region *const dest,
	const node *node)
:	m_event_processor(evp),
	m_src(src?src->activate():NULL),
	m_dest(dest->activate(evp, node)),
	m_node(node),
	m_readdone(NULL),
	m_playdone(NULL),
	m_refcount(1)
{
	m_readdone = new readdone_callback(this, &lib::active_renderer::readdone, 
					new lib::detail::readdone_callback_arg());
}

void
lib::active_renderer::start(lib::event *playdone)
{
	if (!m_node) abort();
	m_playdone = playdone;
	std::ostringstream os;
	os << *m_node;
	lib::logger::get_logger()->trace("active_renderer.start(0x%x, %s, playdone=0x%x)", (void *)this, os.str().c_str(), (void *)playdone);
	m_dest->show(this);
	if (m_src) {
		m_src->start(m_event_processor, m_readdone);
	} else {
		lib::logger::get_logger()->trace("active_renderer.start: no datasource");
		if (m_playdone)
			m_event_processor->add_event(m_playdone, 0, event_processor::low);
	}
}

void
lib::active_renderer::readdone(lib::detail::readdone_callback_arg *dummy)
{
	lib::logger::get_logger()->trace("active_renderer.readdone(0x%x, size=%d)", (void *)this, m_src->size());
	m_dest->need_redraw();
	if (m_playdone)
		m_event_processor->add_event(m_playdone, 0, event_processor::low);
}

void
lib::active_renderer::redraw(const screen_rect<int> &r)
{
	lib::logger::get_logger()->trace("active_renderer.redraw(0x%x)", (void *)this);
}

void
lib::active_renderer::stop()
{
	// XXXX Need to handle case that no data (or not all data) has come in yet
	m_dest->done();
	lib::logger::get_logger()->trace("active_renderer.stop(0x%x)", (void *)this);
}
