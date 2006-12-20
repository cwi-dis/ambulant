// This file is part of Ambulant Player, www.ambulantplayer.org.
//
// Copyright (C) 2003-2005 Stichting CWI, 
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

#include "ambulant/smil2/time_sched.h"
#include "ambulant/smil2/time_node.h"
#include "ambulant/lib/logger.h"
#include <map>

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

static scheduler::time_type infinity = 
std::numeric_limits<scheduler::time_type>::max();

scheduler::scheduler(time_node *root, lib::timer_control *timer)
:	m_root(root), 
	m_timer(timer), 
	m_horizon(0),
	m_locked(false) {
}

scheduler::~scheduler() {
	// m_root is a borrowed ref
	// m_timer is a borrowed ref
}

// Starts a hyperlink target node
void scheduler::start(time_node *tn) {
	lock();
	m_timer->pause();
	if(!m_root->is_active()) {
		m_horizon = 0;
		m_timer->set_time(m_horizon);
		m_root->start();
		if(tn == m_root) {
			m_timer->resume();
			unlock();
			return;
		}
	}
	time_node_context *ctx = m_root->get_context();
	bool oldflag = ctx->wait_for_eom();
	ctx->set_wait_for_eom(false);
	
	const time_node::interval_type& i = tn->get_first_interval(true);
	if(i.is_valid()) goto_previous(tn);
	else goto_next(tn);
	
	ctx->set_wait_for_eom(oldflag);
	m_timer->resume();
	unlock();
}

void scheduler::update_horizon(time_type t) {
	m_horizon = std::max(m_horizon, t);
}

// Activates a node that has a valid scheduled 
// interval after the current time. 
void scheduler::activate_node(time_node *tn) {
	timer::time_type next = m_timer->elapsed();
	while(m_root->is_active() && !tn->is_active()) {
		AM_DBG lib::logger::get_logger()->debug("scheduler:::activate_node:(%s) next=%d ", tn->get_sig().c_str(), next);
		next = _exec(next);
		if(next == infinity) break;
	}
	AM_DBG lib::logger::get_logger()->debug("scheduler::activate_node(%s): leave next=%d tn->is_active %d", tn->get_sig().c_str(), next, tn->is_active());
}

// Starts a hyperlink target that has not played yet. 
void scheduler::goto_next(time_node *tn) {
	// get the path to the time node we want to activate
	// the first in the list is the root and the last is the target
	std::list<time_node*> tnpath;
	tn->get_path(tnpath);
	std::list<time_node*>::iterator it, cit;
	for(it = tnpath.begin(); it != tnpath.end();it++) {
		time_node *parent = *it; cit = it;
		time_node *child = *++cit;
		assert(cit != tnpath.end());
		AM_DBG lib::logger::get_logger()->debug("goto_next: parent=%s child=%s", parent->get_sig().c_str(), child->get_sig().c_str());
		if(parent->is_seq()) activate_seq_child(parent, child);
		else if(parent->is_par()) activate_par_child(parent, child);
		else if(parent->is_excl()) activate_excl_child(parent, child);
		else activate_media_child(parent, child);
		if(child == tnpath.back()) break;
	}
	AM_DBG lib::logger::get_logger()->debug("goto_next: finished");
}

// Starts a hyperlink target that has played. 
void scheduler::goto_previous(time_node *tn) {
	// restart root
	_reset_document();
	m_root->start();
	if(tn == m_root) return;
	
	// get the path to the time node we want to activate
	// the first in the list is the root and the last is the target
	std::list<time_node*> tnpath;
	tn->get_path(tnpath);
	std::list<time_node*>::iterator it, cit;
	for(it = tnpath.begin(); it != tnpath.end();it++) {
		time_node *parent = *it; cit = it;
		time_node *child = *++cit;
		if(parent->is_seq()) activate_seq_child(parent, child);
		else if(parent->is_par()) activate_par_child(parent, child);
		else if(parent->is_excl()) activate_excl_child(parent, child);
		else activate_media_child(parent, child);
		if(child == tnpath.back()) break;
	}
}

// Activate the desinated child starting from the current time
void scheduler::activate_seq_child(time_node *parent, time_node *child) {
	if(child->is_active()) return;
	std::list<time_node*> children;
	parent->get_children(children);
	std::list<time_node*>::iterator it, beginit;
	
	// locate first active
	for(it = children.begin(); it != children.end() && !(*it)->is_active(); it++) {
		AM_DBG lib::logger::get_logger()->debug("activate_seq_child: skip inactive %s", (*it)->get_sig().c_str());
	}
	beginit = it;
	
	for(it = beginit; it != children.end(); it++) {
		if(*it != child) {
			set_ffwd_mode(*it, true);
			AM_DBG lib::logger::get_logger()->debug("activate_seq_child: ffwd earlier %s", (*it)->get_sig().c_str());
		} else {
			break;
		}
	}
	for(it = beginit; it != children.end(); it++) {
		time_node *itt = *it;
		AM_DBG lib::logger::get_logger()->debug("activate_seq_child: activate %s", (*it)->get_sig().c_str());
		activate_node(itt);
		// XXXJack: why is this not always true??? assert(itt->is_active());
		if(itt == child) break;
	}
	for(it = beginit; it != children.end(); it++) {
		if(*it != child) set_ffwd_mode(*it, false);
		else break;
	}
}

// Activate the desinated child starting from the current time
void scheduler::activate_par_child(time_node *parent, time_node *child) {
	activate_node(child);
}

// Activate the desinated child starting from the current time
void scheduler::activate_excl_child(time_node *parent, time_node *child) {
	if(child->is_active()) return;
	child->start();
}

// Activate the desinated child starting from the current time
void scheduler::activate_media_child(time_node *parent, time_node *child) {
	activate_node(child);
}

// Restarts the node
void scheduler::restart(time_node *tn) {
	if(!tn->is_active()) {
		lib::logger::get_logger()->debug("Restart while not active");
		return;
	}
	const time_node::interval_type& i = tn->get_current_interval();
	time_node::time_type bt = i.begin;
	q_smil_time b(tn->sync_node(), bt);
	AM_DBG lib::logger::get_logger()->debug("scheduler::restart: horizon %d -> %d", m_horizon, b.as_doc_time()());
	m_horizon = b.as_doc_time()();
	m_timer->set_time(m_horizon);
	tn->get_timer()->set_time(0);
}

// Executes all the current events
// Returns the time of the next event or the sampling resolution 
scheduler::time_type scheduler::exec() {
	if(locked()) return idle_resolution;
	lock();
	scheduler::time_type rv = _exec();
	unlock();
	return rv;
}

scheduler::time_type scheduler::_exec() {
	time_type now = m_timer->elapsed();
	time_type next = _exec(now);
#if 1
	// This line was taken out a long time ago (rev 1.10, dec 2004) because it seemed to be
	// soaking up CPU cycles. I'm now putting it back in, tentatively, to see if that fixes
	// the seek problems we have:
	// - After a seek some documents will end immediately
	// - Some documents cannot seek, if you try it they'll start at the beginning anyway.
	while(next == now) next = _exec(now);
#endif
	time_type waitdur = next - now;
	AM_DBG lib::logger::get_logger()->debug("scheduler::_exec() done, waitdur=%d, idle_resolution=%d", waitdur, idle_resolution);
	if (waitdur < 0) waitdur = 0;
	return waitdur>idle_resolution?idle_resolution:waitdur;
}

// Executes some of the current events
// Returns the time of the next event or infinity 
scheduler::time_type scheduler::_exec(time_type now) {
	assert(locked());
	time_type next = infinity;
	typedef std::map<time_node::time_type, std::list<time_node*> > event_map_t;
	event_map_t events;
	// events.clear();
	if(!m_root->is_active())
		return next;
	// get all the pending events from the timegraph
	m_root->get_pending_events(events);
	if(events.empty())
		return next;
	event_map_t::iterator eit = events.begin();
	next = (*eit).first();
	std::list<time_node*>& elist = (*eit).second;
	AM_DBG lib::logger::get_logger()->debug("scheduler::_exec: now=%d next=%d got %d events", now, next, events.size());
	next = std::max(m_horizon, next);
	if(now >= next) {
		time_traits::qtime_type timestamp(m_root, next);
		std::list<time_node*>::iterator nit;
		for(nit=elist.begin();nit!=elist.end();nit++) {
			time_node *nitp = *nit;
			nitp->exec(timestamp);
		}
		AM_DBG lib::logger::get_logger()->debug("scheduler::_exec: horizon %d -> %d", m_horizon, next);
		m_horizon = next;
		if(m_timer) m_timer->set_time(next);
		eit++;
		if(eit != events.end()) {
			next = (*eit).first();
			next = std::max(m_horizon, next);
		}
	}
	return next;
}

// Sets the fast forward flag of the time node branch 
void scheduler::set_ffwd_mode(time_node *tn, bool b) {
	time_node::iterator it;
	time_node::iterator end = tn->end();
	for(it=tn->begin(); it!=end; it++) {
		if((*it).first) (*it).second->set_ffwd_mode(b);
	}
}

// Resets the document
void scheduler::reset_document() {
	lock();
	_reset_document();
	unlock();
}

void scheduler::_reset_document() {
	assert(locked());
	time_node::iterator nit;
	time_node::iterator end = m_root->end();
	for(nit=m_root->begin(); nit != end; nit++) {
		if(!(*nit).first) (*nit).second->reset();
	}
	m_horizon = 0;
	m_timer->set_time(m_horizon);
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

// static
std::string scheduler::get_state_sig(time_node *tn) {
	time_node::iterator it;
	time_node::iterator end = tn->end();
	std::string sig;
	for(it = tn->begin(); it != end; it++) {
		if((*it).first) sig += (*it).second->get_state()->sig();
	}
	return sig;
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
	shed.lock(); // XXXJack: correct?
	while(tn->is_active() && next != infinity)
		next = shed._exec(next);
	shed.unlock(); // XXXJack: correct?
	bool finished = tn->get_state()->sig() == 'c';
	reset(tn);
	set_context(tn, oldctx);
	delete algoctx;
	return finished;
}

void
scheduler::lock()
{
	if (m_locked) lib::logger::get_logger()->debug("scheduler::lock(): potential deadlock ahead");
	m_lock.enter();
	assert(!m_locked);
	m_locked = true;
}

void
scheduler::unlock()
{
	assert(m_locked);
	m_locked = false;
	m_lock.leave();
}

