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
	time_type waitdur = do_exec();
	while(waitdur == 0) waitdur = do_exec();
	return waitdur>idle_resolution?idle_resolution:waitdur;
}

scheduler::time_type scheduler::do_exec() {
	time_type waitdur = idle_resolution;
	m_events.clear();
	if(!m_root->is_active())
		return waitdur;
	get_pending_events();
	if(m_events.empty())
		return waitdur;
	event_map_t::iterator eit = m_events.begin();
	time_type next = (*eit).first();
	std::list<time_node*>& elist = (*eit).second;
	next = std::max(m_events_horizon, next);
	if(m_timer->elapsed() >= next) {
		time_traits::qtime_type timestamp(m_root, next);
		std::list<time_node*>::iterator nit;
		for(nit=elist.begin();nit!=elist.end();nit++)
			(*nit)->exec(timestamp);
		m_events_horizon = next;
		m_timer->set_time(next);
		eit++;
		if(eit != m_events.end()) {
			next = (*eit).first();
			next = std::max(m_events_horizon, next);
		}
	}
	waitdur = m_timer->elapsed() - next;
	return waitdur;
}

void scheduler::get_pending_events() {
	time_node::iterator it;
	time_node::iterator end = m_root->end();
	for(it=m_root->begin(); it != end; it++) {
		if((*it).first) (*it).second->get_pending_events(m_events);
	}
}


