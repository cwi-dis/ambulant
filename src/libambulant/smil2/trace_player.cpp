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

#include "ambulant/lib/document.h"
#include "ambulant/lib/node.h"
#include "ambulant/common/trace_playable.h"
#include "ambulant/common/schema.h"
#include "ambulant/smil2/trace_player.h"
#include "ambulant/smil2/timegraph.h"

// A non-gui player for testing scheduler features.

using namespace ambulant;
using namespace smil2;

trace_player::trace_player(lib::document *doc)
:	m_doc(doc),
	m_root(0),
	m_timer(new lib::timer_control_impl(lib::realtime_timer_factory())),
	m_event_processor(0) {
	m_logger = logger::get_logger();
	m_event_processor = lib::event_processor_factory(m_timer);
	timegraph tg(this, m_doc, schema::get_instance());
	m_root = tg.detach_root();
}

trace_player::~trace_player() {
	std::map<const node*, playable *>::iterator it;
	for(it = m_playables.begin();it!=m_playables.end();it++) {
		(*it).second->stop();
		delete (*it).second;
	}
	delete m_root;
	delete m_event_processor;
	delete m_timer;
	delete m_doc;
}

void trace_player::schedule_event(event *ev, lib::timer::time_type t, event_priority ep) {
	m_event_processor->add_event(ev, t, ep);
}

void trace_player::start() {
	m_logger->trace("Started playing");
	m_root->start();
}

void trace_player::stop() {
	m_root->stop();
	m_timer->stop();
	m_logger->trace("Stopped playing");
}

void trace_player::pause() {
	m_timer->pause();
	// we don't propagate pause/resume yet
	std::map<const node*, playable *>::iterator it;
	for(it = m_playables.begin();it!=m_playables.end();it++)
		(*it).second->pause();
}

void trace_player::resume() {
	m_timer->resume();
	// we don't propagate pause/resume yet
	std::map<const node*, playable *>::iterator it;
	for(it = m_playables.begin();it!=m_playables.end();it++)
		(*it).second->resume();
}

bool trace_player::is_done() {
	return !m_root->is_active();
}

void trace_player::start_playable(const node *n, double t, const lib::transition_info *trans) {
	playable *p = get_playable(n);
	if(p) {
		p->start(t);
		return;
	}
	p = new trace_playable(n, n->get_numid());
	m_playables[n] = p;
	p->start(t);
}

void trace_player::stop_playable(const node *n) {
	std::map<const node*, playable *>::iterator it = m_playables.find(n);
	if(it != m_playables.end()) {
		(*it).second->stop();
		delete (*it).second;
		m_playables.erase(it);
	}
}

void trace_player::pause_playable(const node *n, common::pause_display d) {
	playable *p = get_playable(n);
	if(p) p->pause(d);
}

void trace_player::resume_playable(const node *n) {
	playable *p = get_playable(n);
	if(p) p->resume();
}

common::duration
trace_player::get_dur(const node *n) {
	playable *p = get_playable(n);
	if(p) return p->get_dur();
	return common::duration(false, 0);
}

common::playable *trace_player::get_playable(const lib::node *n) {
	std::map<const node*, playable *>::iterator it =
		m_playables.find(n);
	return (it != m_playables.end())?(*it).second:0;
}
