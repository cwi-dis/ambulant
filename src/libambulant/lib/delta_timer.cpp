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

#include "ambulant/lib/delta_timer.h"

#include "ambulant/lib/logger.h"

#if !defined(AMBULANT_NO_IOSTREAMS) && !defined(AMBULANT_NO_STRINGSTREAM)
#include <sstream>
#endif

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;

// A scheduler for timeout events
// Uses delta timer pattern

lib::delta_timer::delta_timer(abstract_timer *t)
:   m_last_run(t->elapsed()),
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
	AM_DBG  if(!queue.empty()) write_trace();
}

#if !defined(AMBULANT_NO_IOSTREAMS) && !defined(AMBULANT_NO_STRINGSTREAM)
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
#endif
