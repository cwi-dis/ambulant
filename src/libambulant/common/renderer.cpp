/*
 * 
 * This file is part of Ambulant Player, www.ambulantplayer.org.
 * 
 * Copyright (C) 2003 Stiching CWI, 
 * Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
 * 
 * Ambulant Player is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Ambulant Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Ambulant Player; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * In addition, as a special exception, if you link Ambulant Player with
 * other files to produce an executable, this library does not by itself
 * cause the resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other reason why
 * the executable file might be covered by the GNU General Public License.
 * 
 * As a special exception, the copyright holders of Ambulant Player give
 * you permission to link Ambulant Player with independent modules that
 * communicate with Ambulant Player solely through the region and renderer
 * interfaces, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting combined work under
 * terms of your choice, provided that every copy of the combined work is
 * accompanied by a complete copy of the source code of Ambulant Player
 * (the version of Ambulant Player used to produce the combined work),
 * being distributed under the terms of the GNU General Public License plus
 * this exception.  An independent module is a module which is not derived
 * from or based on Ambulant Player.
 * 
 * Note that people who make modified versions of Ambulant Player are not
 * obligated to grant this special exception for their modified versions;
 * it is their choice whether to do so.  The GNU General Public License
 * gives permission to release a modified version without this exception;
 * this exception also makes it possible to release a modified version
 * which carries forward this exception. 
 * 
 */

/* 
 * @$Id$ 
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


