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

#include "ambulant/lib/document.h"
#include "ambulant/lib/node.h"
#include "ambulant/common/trace_player.h"
#include "ambulant/common/trace_playable.h"
#include "ambulant/common/schema.h"
#include "ambulant/smil2/timegraph.h"

// A non-gui player for testing scheduler features.

using namespace ambulant;

lib::trace_player::trace_player(document *doc)
:	m_doc(doc),
	m_root(0),
	m_timer(new timer(realtime_timer_factory())),
	m_event_processor(0) {
	m_logger = logger::get_logger();
	m_event_processor = event_processor_factory(m_timer);
	timegraph tg(this, m_doc, schema::get_instance());
	m_root = tg.detach_root();
}

lib::trace_player::~trace_player() {
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

void lib::trace_player::schedule_event(event *ev, time_type t, event_priority ep) {
	m_event_processor->add_event(ev, t, (event_processor::event_priority)ep);
}

void lib::trace_player::start() {
	m_logger->trace("Started playing");
	m_root->start();
}

void lib::trace_player::stop() {
	m_root->stop();
	m_timer->stop();
	m_logger->trace("Stopped playing");
}

void lib::trace_player::pause() {
	m_timer->pause();
	// we don't propagate pause/resume yet
	std::map<const node*, playable *>::iterator it;
	for(it = m_playables.begin();it!=m_playables.end();it++)
		(*it).second->pause();
}

void lib::trace_player::resume() {
	m_timer->resume();
	// we don't propagate pause/resume yet
	std::map<const node*, playable *>::iterator it;
	for(it = m_playables.begin();it!=m_playables.end();it++)
		(*it).second->resume();
}

bool lib::trace_player::is_done() {
	return !m_root->is_active();
}

void lib::trace_player::start_playable(const node *n, double t) {
	playable *p = get_playable(n);
	std::map<const node*, playable *>::iterator it = 
		m_playables.find(n);
	if(p) {
		p->start(t);
		return;
	}
	p = new trace_playable(n, n->get_numid());
	m_playables[n] = p;
	p->start(t);	
}

void lib::trace_player::stop_playable(const node *n) {
	std::map<const node*, playable *>::iterator it = 
		m_playables.find(n);
	if(it != m_playables.end()) {
		(*it).second->stop();
		delete (*it).second;
		m_playables.erase(it);
	}
}

void lib::trace_player::pause_playable(const node *n) {
	playable *p = get_playable(n);
	if(p) p->pause();
}

void lib::trace_player::resume_playable(const node *n) {
	playable *p = get_playable(n);
	if(p) p->resume();
}

std::pair<bool, double> 
lib::trace_player::get_dur(const node *n) {
	playable *p = get_playable(n);
	if(p) return p->get_dur();
	return std::pair<bool, double>(false, 0);
}

lib::playable *lib::trace_player::get_playable(const node *n) {
	std::map<const node*, playable *>::iterator it = 
		m_playables.find(n);
	return (it != m_playables.end())?(*it).second:0;
}
