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

#include "ambulant/smil2/time_sched.h"
#include "ambulant/smil2/time_node.h"


#include "ambulant/lib/logger.h"

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

static scheduler::time_type infinity = 
std::numeric_limits<scheduler::time_type>::max();

scheduler::scheduler(time_node *root, lib::timer *timer)
:	m_root(root), 
	m_timer(timer), 
	m_events_horizon(0) {
}

scheduler::~scheduler() {
	// m_root is a borrowed ref
	// m_timer is a borrowed ref
}

scheduler::time_type scheduler::exec() {
	time_type now = m_timer->elapsed();
	time_type next = exec(now);
	while(next == now) next = exec(now);
	time_type waitdur = next - now;
	return waitdur>idle_resolution?idle_resolution:waitdur;
}

scheduler::time_type scheduler::exec(time_type now) {
	time_type next = infinity;
	m_events.clear();
	if(!m_root->is_active())
		return next;
	get_pending_events();
	if(m_events.empty())
		return next;
	event_map_t::iterator eit = m_events.begin();
	next = (*eit).first();
	std::list<time_node*>& elist = (*eit).second;
	next = std::max(m_events_horizon, next);
	if(now >= next) {
		time_traits::qtime_type timestamp(m_root, next);
		std::list<time_node*>::iterator nit;
		for(nit=elist.begin();nit!=elist.end();nit++)
			(*nit)->exec(timestamp);
		m_events_horizon = next;
		if(m_timer) m_timer->set_time(next);
		eit++;
		if(eit != m_events.end()) {
			next = (*eit).first();
			next = std::max(m_events_horizon, next);
		}
	}
	return next;
}

void scheduler::get_pending_events() {
	time_node::iterator it;
	time_node::iterator end = m_root->end();
	for(it=m_root->begin(); it != end; it++) {
		if((*it).first) (*it).second->get_pending_events(m_events);
	}
}

void scheduler::reset() {
	time_traits::qtime_type timestamp(m_root, 0);
	m_root->reset(timestamp, m_root);
	m_events_horizon = 0;
	if(m_timer) {
		m_timer->pause();
		m_timer->set_time(0);
	}
}

// static
void scheduler::reset(time_node *tn) {
	time_traits::qtime_type timestamp(tn, 0);
	tn->reset(timestamp, tn);
}

// static
void scheduler::set_context(time_node *tn, time_node_context *ctx) {
	time_node::iterator it;
	time_node::iterator end = tn->end();
	for(it=tn->begin(); it != end; it++) {
		if((*it).first) (*it).second->set_context(ctx);
	}
}

// Returns true when the document will reach its end without requiring events
// This does not mean that has not event timing. Events may affect playback.
// static
bool scheduler::has_resolved_end(time_node *tn) {
	time_node_context *oldctx = tn->get_context();
	time_node_context *algoctx = new dummy_time_node_context();
	set_context(tn, algoctx);
	reset(tn);
	scheduler shed(tn, 0);
	tn->start();
	timer::time_type next = 0;
	while(tn->is_active() && next != infinity)
		next = shed.exec(next);
	bool finished = tn->get_state()->sig() == 'c';
	reset(tn);
	set_context(tn, oldctx);
	delete algoctx;
	return finished;
}

