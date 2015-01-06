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

#include "ambulant/lib/delta_timer.h"

#include "ambulant/lib/logger.h"

#include <sstream>

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

// A scheduler for timeout events
// Uses delta timer pattern

lib::delta_timer::delta_timer(timer *t)
:	m_last_run(t->elapsed()),
	m_timer(t) {}

lib::delta_timer::~delta_timer() {
	std::list<timeout_event>::iterator it;
	for(it = m_events.begin(); it != m_events.end(); it++)
		delete (*it).first;
}

// called periodically
// fires ready events
void lib::delta_timer::execute() {
	time_type now = m_timer->elapsed();
	time_type delta = now - m_last_run;
	m_last_run = now;
	fire_delta_events(delta);
}

void lib::delta_timer::clear() {
	std::list<timeout_event>::iterator it;
	for(it = m_events.begin(); it != m_events.end(); it++)
		delete (*it).first;
	m_events.clear();
}

// called periodically
// returns the events that are ready to fire
void lib::delta_timer::execute(std::queue<event*>& queue) {
	time_type now = m_timer->elapsed();
	time_type delta = now - m_last_run;
	m_last_run = now;
	get_ready_delta_events(delta, queue);
}

void lib::delta_timer::insert(event *pe, time_type t) {
	if(m_events.size() == 0){
		m_events.push_back(timeout_event(pe, t));
	} else {
		// find correct pos (do: pe->incr(-(*i).get_time() as we traverse the list)
		std::list<timeout_event>::iterator it;
		for(it = m_events.begin();it!=m_events.end();it++) {
			if(t < (*it).second)
				break;
			decr(t, (*it).second);
		}
		// insert entry
		if(it != m_events.end()) {
			decr((*it).second, t);
			m_events.insert(it, timeout_event(pe, t));
		} else {
			m_events.push_back(timeout_event(pe, t));
		}
	}
	// debug
	AM_DBG write_trace();
}

// Cancels an event
bool lib::delta_timer::cancel(event *pe) {
	if(m_events.size() == 0) return false;
	std::list<timeout_event>::iterator it;
	for(it = m_events.begin();it!=m_events.end();it++)
		if((*it).first == pe) break;
	if(it == m_events.end()) return false;

	// the iterator is positioned at the event we want to cancel
	time_type dt = (*it).second;
	delete (*it).first;
	it = m_events.erase(it);

	// the iterator is positioned at the next event
	if(it != m_events.end())
		(*it).second += dt;
	return true;
}

void lib::delta_timer::fire_delta_events(time_type delta) {
	while(m_events.size()>0 && m_events.front().second <= delta) {
		decr(delta, m_events.front().second);
		m_events.front().first->fire();
		delete m_events.front().first;
		m_events.erase(m_events.begin());
	}
	if(m_events.size()>0)
		decr(m_events.front().second, delta);
}

void lib::delta_timer::get_ready_delta_events(time_type delta, std::queue<event*>& queue) {
	while(m_events.size()>0 && m_events.front().second <= delta) {
		decr(delta, m_events.front().second);
		queue.push(m_events.front().first);
		m_events.erase(m_events.begin());
	}
	if(m_events.size()>0)
		decr(m_events.front().second, delta);

	// debug
	AM_DBG	if(!queue.empty()) write_trace();
}

lib::timer::time_type lib::delta_timer::next_event_time() const
{
	std::list<timeout_event>::const_iterator it = m_events.begin();
	if (it == m_events.end())
		return 1000000;
	return (*it).second;
}


void lib::delta_timer::write_trace() {
	std::ostringstream os;
	os << "delta_timer (";
	std::list<timeout_event>::iterator it = m_events.begin();
	if(it != m_events.end()) {
		os << (*it).second;
		it++;
	}
	for(; it != m_events.end(); it++)
		os << ", " << (*it).second;
	os << ")";
	lib::logger::get_logger()->trace(os.str());
}
