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
#include "ambulant/gui/none/none_gui.h"

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace common;

typedef lib::no_arg_callback<active_renderer> readdone_callback;

active_renderer::active_renderer(
	active_playable_events *context,
	active_playable_events::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp,
	net::passive_datasource *src,
	abstract_rendering_surface *const dest)
:	active_basic_renderer(context, cookie, node, evp),
	m_src(src?src->activate():NULL),
	m_dest(dest)
{
}

void
active_renderer::start(double t)
{
#ifndef AMBULANT_NO_ABORT
	if (!m_node) abort();
#endif

#if !defined(AMBULANT_NO_IOSTREAMS) && !defined(AMBULANT_NO_STRINGSTREAM)
	std::ostringstream os;
	os << *m_node;
	AM_DBG lib::logger::get_logger()->trace("active_renderer.start(0x%x, %s)", (void *)this, os.str().c_str());
#endif
	m_dest->show(this);
	if (m_src) {
		lib::event *e = new readdone_callback(this, &active_renderer::readdone);
		m_src->start(m_event_processor, e);
	} else {
		lib::logger::get_logger()->error("active_renderer.start: no datasource");
		stopped_callback();
	}
}

void
active_renderer::readdone()
{
	AM_DBG lib::logger::get_logger()->trace("active_renderer.readdone(0x%x, size=%d)", (void *)this, m_src->size());
	m_dest->need_redraw();
	stopped_callback();
}

void
active_renderer::stop()
{
	// XXXX Need to handle case that no data (or not all data) has come in yet
	m_dest->renderer_done();
	AM_DBG lib::logger::get_logger()->trace("active_renderer.stop(0x%x)", (void *)this);
}

void
active_renderer::wantclicks(bool want)
{
	m_dest->need_events(want);
}

active_final_renderer::~active_final_renderer()
{
	if (m_data) free(m_data);
}

void
active_final_renderer::readdone()
{
	AM_DBG lib::logger::get_logger()->trace("active_final_renderer.readdone(0x%x, size=%d)", (void *)this, m_src->size());
	m_data_size = m_src->size();
	if ((m_data = malloc(m_data_size)) == NULL) {
		lib::logger::get_logger()->fatal("active_final_renderer.readdone: cannot allocate %d bytes", m_data_size);
#ifndef AMBULANT_NO_ABORT
		abort();
#endif
	}
	m_src->read((char *)m_data, m_data_size);
	m_dest->need_redraw();
	stopped_callback();
}

global_renderer_factory::global_renderer_factory()
:   m_default_factory(new gui::none::none_renderer_factory())
{
}

global_renderer_factory::~global_renderer_factory()
{
    // XXXX Should I delete the factories in m_factories? I think
    // so, but I'm not sure...
    delete m_default_factory;
}
    
void
global_renderer_factory::add_factory(renderer_factory *rf)
{
    m_factories.push_back(rf);
}
    
active_basic_renderer *
global_renderer_factory::new_renderer(
	active_playable_events *context,
	active_playable_events::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *const evp,
	net::passive_datasource *src,
	abstract_rendering_surface *const dest)
{
    std::vector<renderer_factory *>::iterator i;
    active_basic_renderer *rv;
    
    for(i=m_factories.begin(); i != m_factories.end(); i++) {
        rv = (*i)->new_renderer(context, cookie, node, evp, src, dest);
        if (rv) return rv;
    }
    return m_default_factory->new_renderer(context, cookie, node, evp, src, dest);
}
