// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2015 Stichting CWI, 
// Science Park 123, 1098 XG Amsterdam, The Netherlands.
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

#include "ambulant/config/config.h"
#include "ambulant/lib/logger.h"
#include "ambulant/lib/transition_info.h"
#include "ambulant/common/renderer_impl.h"
#include "ambulant/gui/none/none_gui.h"
#include "ambulant/net/datasource.h"
#include "ambulant/lib/parselets.h"
#include "ambulant/smil2/params.h"


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
	lib::event_processor* evp,
	common::factories *fp,
	common::playable_factory_machdep *mdp)
:	playable_imp(context, cookie, node, evp, fp, mdp),
	m_dest(0),
	m_alignment(0),
	m_activated(false),
	m_erase_never(false),
	m_clip_begin(0),
	m_clip_end(-1)

{
	AM_DBG lib::logger::get_logger()->debug("renderer_playable(%s, cookie=%d)", node->get_sig().c_str(), (int)cookie);
}

renderer_playable::~renderer_playable()
{
	if (m_dest) m_dest->renderer_done(this);
	m_dest = NULL;
}

void
renderer_playable::init_with_node(const lib::node *n)
{
	m_node = n;
	m_cookie = m_node->get_numid();
	_init_clip_begin_end();
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
		return;
	}
	m_dest->show(this);
}

bool
renderer_playable::stop()
{
	AM_DBG lib::logger::get_logger()->debug("renderer_playable.stop(0x%x)", (void *)this);
	if (!m_activated) {
		lib::logger::get_logger()->trace("renderer_playable.stop(0x%x): not started", (void*)this);
	} else {
		if (m_dest) m_dest->renderer_done(this);
		m_dest = NULL;
	}
	m_activated = false;
	return true;
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
	net::timestamp_t cb = 0;

	// here we have to get clip_begin/clip_end from the node
	const char *clip_begin_attr = m_node->get_attribute("clipBegin");
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
			lib::logger::get_logger()->warn(gettext("Cannot parse %s"),"clipBegin");
		} else {
			cb += (net::timestamp_t)parser.get_time() * 1000;
			AM_DBG lib::logger::get_logger()->debug("parsed clipBegin cb=%lld", cb);

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
			lib::logger::get_logger()->warn(gettext("Cannot parse %s"),"clipEnd");
		} else {
			ce = (net::timestamp_t)parser.get_time() * 1000;
		}
	}
	AM_DBG lib::logger::get_logger()->debug("renderer_playable::init_clip_begin_end: cb=%lld, ce=%lld", cb,ce);
	if (cb < 0) {
		lib::logger::get_logger()->trace("%s: negative clipBegin value (%s) ignored",  m_node->get_sig().c_str(), clip_begin_attr);
		cb = 0;
	}
	if (ce != -1 && ce < cb) {
		lib::logger::get_logger()->trace("%s: clipEnd=\"%s\" is before clipBegin=\"%s\", media ignored",  m_node->get_sig().c_str(),  clip_end_attr, clip_begin_attr);
	}
	m_clip_begin = cb;
	m_clip_end = ce;
}

renderer_playable_ds::renderer_playable_ds(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	common::factories *factory,
	common::playable_factory_machdep *mdp)
:	renderer_playable(context, cookie, node, evp, factory, mdp),
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
		m_src->stop();
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
		m_context->started(m_cookie, 0);
		m_src->start(m_event_processor, e);
	} else {
		lib::logger::get_logger()->trace("renderer_playable_ds.start: no datasource, skipping media item");
		m_context->started(m_cookie, 0);
		m_context->stopped(m_cookie, 0);
	}
}

void
renderer_playable_ds::seek(double t)
{
	lib::logger::get_logger()->trace("renderer_playable_ds: seek(%f) not implemented", t);
}

bool
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
	return true;
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
    {
        // Get bandwidth usage data
        const char *resource;
        long bw_amount = m_src->get_bandwidth_usage_data(&resource);
        if (bw_amount >= 0) 
            m_context->playable_resource(this, resource, bw_amount);
    }
	size_t cur_size = m_src->size();
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_dsall.readdone(0x%x, size=%d) cookie=%d", (void *)this, cur_size, m_cookie);

	if (!m_partial_data)
		m_partial_data = malloc(cur_size+1);
	else
		m_partial_data = realloc(m_partial_data, m_partial_data_size + cur_size + 1);

	if (m_partial_data == NULL) {
		lib::logger::get_logger()->fatal("renderer_playable_dsall.readdone: cannot allocate %d bytes", cur_size);
		// What else can we do...
		m_context->stopped(m_cookie, 0);
		m_src->readdone(cur_size);
		m_src->stop();
		return;
	}
	AM_DBG lib::logger::get_logger()->debug("renderer_playable_dsall.readdone(0x%x): calling m_src->get_read_ptr() m_src=0x%x", (void *)this,m_src);
	char *cur_data = m_src->get_read_ptr();

	memcpy((char *)m_partial_data + m_partial_data_size, cur_data, cur_size);
	m_partial_data_size += cur_size;
	((char*)m_partial_data)[m_partial_data_size] = '\0';
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
:	m_default_factory(new gui::none::none_playable_factory())
{
}

global_playable_factory_impl::~global_playable_factory_impl()
{
	// Clear the renderer selection cache
	std::map<int, renderer_select*>::iterator ri;
	for (ri=m_renderer_select.begin(); ri!=m_renderer_select.end(); ri++)
		delete (*ri).second;
	m_renderer_select.clear();
	// Clear the factories
	delete m_default_factory;
	std::list<playable_factory*>::iterator fi;
	for(fi=m_factories.begin(); fi != m_factories.end(); fi++) {
		playable_factory *pf = *fi;
		delete pf;
	}
	m_factories.clear();
}

void
global_playable_factory_impl::add_factory(playable_factory *rf)
{
	m_factories.push_back(rf);
}

void
global_playable_factory_impl::preferred_renderer(const char* name)
{
	renderer_select rs(name);
	std::list<playable_factory *>::iterator i;
	std::list<playable_factory *> new_list;

	for (i=m_factories.begin(); i!=m_factories.end(); i++) {
		if ((*i)->supports(&rs)) {
			AM_DBG lib::logger::get_logger()->debug("preferred_renderer: moving 0x%x to front", *i);
			new_list.push_back(*i);
		}
	}
	for (i=m_factories.begin(); i!=m_factories.end(); i++) {
		if (!(*i)->supports(&rs))
			new_list.push_back(*i);
	}
	m_factories = new_list;
}

playable *
global_playable_factory_impl::new_playable(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp)
{
	std::list<playable_factory *>::iterator i;

	// First make sure we have the node in our renderer selection cache
	int nid = node->get_numid();
    m_lock.enter();
	if (m_renderer_select.count(nid) == 0) {
		m_renderer_select[nid] = new renderer_select(node);
	}
	renderer_select *rs = m_renderer_select[nid];
    m_lock.leave();
    

	// If we don't have a renderer selected yet select one
	playable *rv = NULL;
	playable_factory *pf = rs->get_playable_factory();
	if (pf) {
		// We have a cached playable factory. Let's use it.
		rv = pf->new_playable(context, cookie, node, evp);
	} else {
		// No cached playable factory. Iterate over all of them,
		// and when we find one that works we remember it.
		for(i=m_factories.begin(); i != m_factories.end(); i++) {
			if ((*i)->supports(rs)) {
				rv = (*i)->new_playable(context, cookie, node, evp);
				if (rv) {
					// This one works! Let's remember it for next time.
					pf = (*i);
					break;
				}
			}
		}
		if (rv == NULL) {
			// We have no factories that can render this node...
			rv = m_default_factory->new_playable(context, cookie, node, evp);
			pf = m_default_factory;
		}
		rs->set_playable_factory(pf);
	}
	return rv;
}

playable *
global_playable_factory_impl::new_aux_audio_playable(
	playable_notification *context,
	playable_notification::cookie_type cookie,
	const lib::node *node,
	lib::event_processor *evp,
	net::audio_datasource *src)
{
	std::list<playable_factory *>::iterator i;
	playable *rv;

	for(i=m_factories.begin(); i != m_factories.end(); i++) {
		rv = (*i)->new_aux_audio_playable(context, cookie, node, evp, src);
		if (rv) return rv;
	}

	return NULL;
}

global_playable_factory *
common::get_global_playable_factory()
{
	return new global_playable_factory_impl();
}
