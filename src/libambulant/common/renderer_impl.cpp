// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2008 Stichting CWI, 
// Kruislaan 413, 1098 SJ Amsterdam, The Netherlands.
//
// Ambulant Player is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// Ambulant Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ambulant Player; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* 
 * @$Id$ 
 */

#include "ambulant/config/config.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/transition_info.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/net/datasource.h"
#include "ambulant/lib/parselets.h"


//#define AM_DBG
#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace common;

typedef lib::no_arg_callback<renderer_playable_ds> readdone_callback;

renderer_playable::renderer_playable(
	playable_notification *context,
	cookie_type cookie,
	const lib::node *node,
	lib::event_processor* evp) 
:	playable_imp(context, cookie, node, evp),
	m_dest(0),
	m_alignment(0),
	m_activated(false),
	m_erase_never(false),
	m_clip_begin(0),
	m_clip_end(-1)

{
	const char *erase = m_node->get_attribute("erase");
	if (erase && strcmp(erase, "never") == 0)
		m_erase_never = true;
}

void
renderer_playable::start(double t)
{
	AM_DBG lib::logger::get_logger()->debug("renderer_playable.start(0x%x)", (void *)this);
	if (m_activated) {
		lib::logger::get_logger()->trace("renderer_playable.start(0x%x): already started", (void*)this);
		return;
	}
	m_activated = true;
	if (!m_dest) {
		lib::logger::get_logger()->trace("renderer_playable.start: no destination surface, skipping media item");
		m_context->stopped(m_cookie, 0);
		return;
	}
	m_dest->show(this);
}

void
renderer_playable::stop()
{
	AM_DBG lib::logger::get_logger()->debug("renderer_playable.stop(0x%x)", (void *)this);
	if (!m_activated) {
		lib::logger::get_logger()->trace("renderer_playable.stop(0x%x): not started", (void*)this);
	} else {
		if (m_dest)
			m_dest->renderer_done(this);
	}
	m_activated = false;
}

bool
renderer_playable::user_event(const lib::point &where, int what) {
	AM_DBG lib::logger::get_logger()->debug("%s: renderer_playable::user_event((%d,%d), %d) -> %d", m_node->get_sig().c_str(), where.x, where.y, what, m_cookie);
	if (!user_event_sensitive(where)) return false;
	if (what == user_event_click) m_context->clicked(m_cookie, 0);
	else if (what == user_event_mouse_over) m_context->pointed(m_cookie, 0);
	else assert(0);
	return true;
}

bool
renderer_playable::user_event_sensitive(const lib::point &where) {
	const char *sensitive = m_node->get_attribute("sensitivity");
	if (sensitive == NULL || strcmp(sensitive, "opaque") == 0) return true;
	if (strcmp(sensitive, "transparent") == 0) return false;
	static bool warned = false;
	if (!warned) {
		warned = true;
		lib::logger::get_logger()->trace("%s: only \"transparent\" and \"opaque\" implemented for sensitive attribute", m_node->get_sig().c_str());
	}
	return true;
}

void
renderer_playable::_init_clip_begin_end()
{
	// here we have to get clip_begin/clip_end from the node
	const char *clip_begin_attr = m_node->get_attribute("clipBegin");
	net::timestamp_t cb = 0;
	
	if (!clip_begin_attr) {
		clip_begin_attr = m_node->get_attribute("clip-begin");
	}
	
	if (clip_begin_attr) {
		lib::mediaclipping_p parser;
		std::string s(clip_begin_attr);
		std::string::const_iterator b = s.begin();
		std::string::const_iterator e = s.end();
		std::ptrdiff_t d = parser.parse(b, e);
		if (d == -1) {
			lib::logger::get_logger()->warn("Cannot parse clipBegin");
		} else {
			cb = (net::timestamp_t)parser.get_time() * 1000;
			// lib::logger::get_logger()->warn("parsed clipBegin cb=%lld", cb);

		}
	}
	
	const char *clip_end_attr = m_node->get_attribute("clipEnd");
	net::timestamp_t ce = -1;
	if (!clip_end_attr) {
		clip_end_attr = m_node->get_attribute("clip-end");
	}
	
	if (clip_end_attr) {
		lib::mediaclipping_p parser;
		std::string s(clip_end_attr);
		std::string::const_iterator b = s.begin();
		std::string::const_iterator e = s.end();
		std::ptrdiff_t d = parser.parse(b, e);
		if (d == -1) {
			lib::logger::get_logger()->warn("Cannot parse clipEnd");
		} else {
			ce = (net::timestamp_t)parser.get_time() * 1000;
		}	
	}
	AM_DBG lib::logger::get_logger()->debug("renderer_playable::init_clip_begin_end: cb=%lld, ce=%lld", cb,ce);
	m_clip_begin = cb;
	m_clip_end = ce;
}

renderer_playable_ds::renderer_playable_ds(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory)
:	renderer_playable(context, cookie, node, evp),
	m_src(NULL)
{
	// XXXX m_src = passive_datasource(node->get_url("src"))->activate()
	net::url url = node->get_url("src");
	m_src = factory->get_datasource_factory()->new_raw_datasource(url);	
}

renderer_playable_ds::~renderer_playable_ds()
{
	AM_DBG lib::logger::get_logger()->debug("~renderer_playable_ds(0x%x)", (void *)this);
  	if (m_src) {
		m_src->release();
		m_src = NULL;
	}
}

void
renderer_playable_ds::start(double t)
{
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_ds.start(0x%x %s)", (void *)this, m_node->get_sig().c_str());
	if (m_activated) {
		lib::logger::get_logger()->trace("renderer_playable_ds.start(0x%x): already started", (void*)this);
		return;
	}
	renderer_playable::start(t);
	if (m_src) {
		lib::event *e = new readdone_callback(this, &renderer_playable_ds::readdone);
		m_src->start(m_event_processor, e);
		m_context->started(m_cookie, 0);
	} else {
		lib::logger::get_logger()->trace("renderer_playable_ds.start: no datasource, skipping media item");
		m_context->stopped(m_cookie, 0);
	}
}

void
renderer_playable_ds::seek(double t)
{
	lib::logger::get_logger()->trace("renderer_playable_ds: seek(%f) not implemented", t);
}

void
renderer_playable_ds::stop()
{
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_ds.stop(0x%x %s)", (void *)this, m_node->get_sig().c_str());
	renderer_playable::stop();
	if (m_src) {
		m_src->stop();
		m_src->release();
	}
	m_src = NULL;
	if (m_context)
		m_context->stopped(m_cookie, 0);
}

renderer_playable_dsall::~renderer_playable_dsall()
{
	if (m_data) free(m_data);
	if (m_partial_data) free(m_partial_data);
}

void
renderer_playable_dsall::readdone()
{
	if (!m_src) return;	
	unsigned cur_size = m_src->size();
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_dsall.readdone(0x%x, size=%d) cookie=%d", (void *)this, cur_size, m_cookie);
	
	if (!m_partial_data)
		m_partial_data = malloc(cur_size);
	else
		m_partial_data = realloc(m_partial_data, m_partial_data_size + cur_size);
	
	if (m_partial_data == NULL) {
		lib::logger::get_logger()->fatal("renderer_playable_dsall.readdone: cannot allocate %d bytes", cur_size);
		// What else can we do...
		m_context->stopped(m_cookie, 0);
	}
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_dsall.readdone(0x%x): calling m_src->get_read_ptr() m_src=0x%x", (void *)this,m_src);
	char *cur_data = m_src->get_read_ptr();

	memcpy((char *)m_partial_data + m_partial_data_size, cur_data, cur_size);
	m_partial_data_size += cur_size;
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_dsall.readdone(0x%x): calling m_src->readdone(%d)", (void *)this,cur_size);
	m_src->readdone(cur_size);
	
	if (m_src->end_of_file()) {
		// All done. Move staging area data to the real place.
		assert(!m_data);
		m_data_size = m_partial_data_size;
		m_data = m_partial_data;
		m_partial_data = NULL;
		AM_DBG lib::logger::get_logger()->debug("renderer_playable_dsall.readdone(0x%x):  all done, calling need_redraw() and stopped_callback", (void *)this);
		if (m_dest)
			m_dest->need_redraw();
		m_src->stop();
		m_context->stopped(m_cookie, 0);
	} else {
		// Continue reading
		AM_DBG lib::logger::get_logger()->debug("renderer_playable_dsall.readdone(0x%x):  more to come, calling m_src->start()", (void *)this);
		lib::event *e = new readdone_callback(this, &renderer_playable_ds::readdone);
		m_src->start(m_event_processor, e);
	}
	
}

global_playable_factory_impl::global_playable_factory_impl()
:   m_default_factory(new gui::none::none_playable_factory())
{
}

global_playable_factory_impl::~global_playable_factory_impl()
{
    // XXXX Should I delete the factories in m_factories? I think
    // so, but I'm not sure...
    std::vector<playable_factory*>::iterator i;
    for(i=m_factories.begin(); i != m_factories.end(); i++)
		delete (*i);
	m_factories.clear();
    delete m_default_factory;
}
    
void
global_playable_factory_impl::add_factory(playable_factory *rf)
{
    m_factories.push_back(rf);
}
    
playable *
global_playable_factory_impl::new_playable(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp)
{
    std::vector<playable_factory *>::iterator i;
    playable *rv;
    
    for(i=m_factories.begin(); i != m_factories.end(); i++) {
        rv = (*i)->new_playable(context, cookie, node, evp);
        if (rv) return rv;
    }
    return m_default_factory->new_playable(context, cookie, node, evp);
}

playable *
global_playable_factory_impl::new_aux_audio_playable(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	net::audio_datasource *src)
{
    std::vector<playable_factory *>::iterator i;
    playable *rv;
    
    for(i=m_factories.begin(); i != m_factories.end(); i++) {
        rv = (*i)->new_aux_audio_playable(context, cookie, node, evp, src);
        if (rv) return rv;
    }
	
    return m_default_factory->new_playable(context, cookie, node, evp);
}

global_playable_factory *
common::get_global_playable_factory()
{
    return new global_playable_factory_impl();
}
