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
#include "ambulant/smil2/time_state.h"
#include "ambulant/smil2/time_node.h"

//#define AM_DBG

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

using namespace ambulant;
using namespace smil2;

///////////////////////
// time_state

// Base class for time states.
// Offers the common time states interface.
// Brings into time states scope, node's common state variables.
//
// The FSM implemented by the time states asserts that on any transition
// the exit() actions of the previous state are always executed 
// and then the enter() actions of the new state 
// (this is true also for self transitions).
//
time_state::time_state(time_node *tn) 
:	m_self(tn),
	m_interval(tn->m_interval),
	m_picounter(tn->m_picounter),
	m_active(tn->m_active),
	m_needs_remove(tn->m_needs_remove),
	m_last_cdur(tn->m_last_cdur),
	m_rad(tn->m_rad),
	m_precounter(tn->m_precounter),
	m_impldur(tn->m_impldur),
	m_attrs(tn->m_attrs){
}

void time_state::sync_update(qtime_type timestamp) {
}

void time_state::kill(qtime_type timestamp, time_node *oproot) {
	m_self->set_state(ts_dead, timestamp, oproot);
}

void time_state::reset(qtime_type timestamp, time_node *oproot) {
	m_self->set_state(ts_reset, timestamp, oproot);
}

void time_state::exit(qtime_type timestamp, time_node *oproot) {
}

void time_state::report_state(qtime_type timestamp) {
	AM_DBG logger::get_logger()->trace("%s[%s] --> %s at PT:%ld, DT:%ld", 
		m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), 
		time_state_str(this->ident()),
		timestamp.second(),
		timestamp.as_doc_time()());
}

///////////////////////
// reset_state

// An element enters this state when an ancestor begins 
// or repeats (e.g. on an ancestor beginEvent or repeat).
// Element's instance lists are reset with respect to that ancestor.
// Therefore resetting an element while in the reset state is meaningful. 
// An element will remain in this state until its parent is activated.
// Before the document starts all elements are in this state.

// Reset enter / update state variables
// Reset do / on parent beginEvent transition to PROACTIVE
// Reset exit / ?

void reset_state::enter(qtime_type timestamp) {
	m_interval = interval_type::unresolved;
	m_picounter = 0;
	m_active = false;
	m_needs_remove = false;
	m_last_cdur = time_type::unresolved;
	m_rad = 0;
	m_precounter = 0;
	m_impldur = time_type::unresolved;
	// Resetting the above variables is only part of the reset process.
	// See time_node::reset()
	//report_state(timestamp);
}

void reset_state::sync_update(qtime_type timestamp) {}

void reset_state::exit(qtime_type timestamp, time_node *oproot) {
	// next is proactive when the parent begins (normal)
	// next is a self-transition to reset when an encestor begins or repeats (reset)
}
	


///////////////////////
// proactive_state

// An element enters this state when its parent begins 
// or repeats (e.g. on parent beginEvent or repeat).
//
// Proactive enter / update state variables, evaluate first interval
// Proactive do / on sync update re-evaluate interval, enter active on timer event
// Proactive exit / ?

void proactive_state::enter(qtime_type timestamp) {
	report_state(timestamp);
	interval_type i = m_self->calc_first_interval();
	if(i.is_valid()) {	
		// Set the calc interval as current and update dependents
		m_self->schedule_interval(timestamp, i);
		// The above call may result 
		// a) to a transition to active if the interval contains timestamp
		// b) to remain proactive waiting for the scheduled interval
		// c) to a jump to postactive if the interval is in the past.
	} 
	// else wait in the current state for new happenings 
}

void proactive_state::sync_update(qtime_type timestamp) {
	if(m_self->deferred()) return;
	interval_type i = m_self->calc_first_interval();
	AM_DBG logger::get_logger()->trace("%s[%s].proactive_state::sync_update %s --> %s at DT:%ld", 
		m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), 
		::repr(m_interval).c_str(),
		::repr(i).c_str(),
		timestamp.as_doc_time()());
	if(m_interval.is_valid() && i.is_valid() && i != m_interval) {
		m_self->update_interval(timestamp, i);
	} else if(m_interval.is_valid() && !i.is_valid()) {
		m_self->cancel_interval(timestamp);
	} else if(!m_interval.is_valid() && i.is_valid()) {
		m_self->schedule_interval(timestamp, i);
	}
}

void proactive_state::kill(qtime_type timestamp, time_node *oproot) {
	// cancels any interval and transitions to dead
	if(m_interval.is_valid())
		m_self->cancel_interval(timestamp);
	m_self->set_state(ts_dead, timestamp, oproot);
}

void proactive_state::reset(qtime_type timestamp, time_node *oproot) {
	// cancels any interval and transitions to reset
	if(m_interval.is_valid())
		m_self->cancel_interval(timestamp);
	m_self->set_state(ts_reset, timestamp, oproot);
}	 

void proactive_state::exit(qtime_type timestamp, time_node *oproot) {
	// next is active when the interval is within parent AD (normal)
	// next is postactive if 
	//		the interval is in the past (jump)
	//		the interval will not be exec because of peers="never" excl semantics
	// next is reset if the parent repeats or restarts (reset)
	// next is dead if the parent ends (kill)
	
}



///////////////////////
// active_state

// An element enters the active state when it has a valid
// interval and begin <= parent->get_simple_time().
// An element will remain in this state until its interval completes
// or its parent ends.
// A node with a valid interval may never enter the active state,
// or, may enter the active state halfway through its interval.
// In the cases that an interval or part of it cannot be played 
// the respective real events will not be raised but
// dependents should be updated for the interval. 
// 
// Active enter / update state variables, raise_begin_event, peer.start(t)?
// Active do /
//	on repeat: m_last_cdur=clock_value, m_rad += m_last_cdur, m_precounter++;
//	on sync update re-evaluate current interval's 'end', consider restart semantics
// Active exit / update state variables, raise_end_event, peer.pause() or peer.stop()
//
// timestamp: "scheduled now" in parent simple time
//
void active_state::enter(qtime_type timestamp) {
	report_state(timestamp);
	m_active = true;
	m_needs_remove = true; 
	
	// if this is in a seq should remove any freeze effect of previous
	if(m_self->up() && m_self->up()->is_seq()) {
		 time_node *prev = m_self->previous();
		 if(prev) {
			const time_attrs* ta = prev->get_time_attrs();
			if(ta->get_fill() == fill_freeze)
				prev->remove(timestamp);
		 }
	}
	
	// The timestamp in parent simple time
	// Children should convert it to their parent
	m_self->reset_children(timestamp, m_self);
	
	m_self->activate(timestamp);
	
	// The timestamp in parent simple time
	// Children should convert it to their parent
	m_self->startup_children(timestamp);
	
	// raise_begin_event async
	m_self->raise_begin_event_async(timestamp);
}

void active_state::sync_update(qtime_type timestamp) {
	// Update end, consider restart semantics
	AM_DBG logger::get_logger()->trace("%s[%s].active_state::sync_update() at ST:%ld PT:%ld, DT:%ld", 
		m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), 
		timestamp.as_time_value_down_to(m_self),
		timestamp.second(),
		timestamp.as_doc_time_value());
	time_type end = m_self->calc_current_interval_end();
	if(end != m_interval.end) {
		m_self->update_interval_end(timestamp, end);
	}
	
	restart_behavior rb = m_attrs.get_restart();
	if(rb == restart_always) {
		interval_type dummy = m_interval;
		m_interval.end = timestamp.second;
		interval_type i = m_self->calc_next_interval();
		m_interval = dummy;
		if(i.is_valid()) {
			AM_DBG logger::get_logger()->trace("%s[%s] restart interval %s",
				m_attrs.get_tag().c_str(), 
				m_attrs.get_id().c_str(),
				::repr(i).c_str()); 
			m_self->set_state(ts_postactive, timestamp, m_self);
			m_self->set_begin_event_inst(timestamp.second);
			m_self->schedule_sync_update(timestamp, 0);
		}
	}
}

void active_state::kill(qtime_type timestamp, time_node *oproot) {
	// Forced transition to dead originated by oproot 
	// The exit() actions will be executed
	// e.g. fill, raise events, update dependents, etc
	m_self->set_state(ts_dead, timestamp, oproot);
	
}

void active_state::reset(qtime_type timestamp, time_node *oproot) {
	// Forced transition to reset originated by oproot 
	// The exit() actions will be executed
	m_self->remove(timestamp);
	m_self->set_state(ts_reset, timestamp, oproot);
	
}	 

void active_state::exit(qtime_type timestamp, time_node *oproot) {
	m_active = false;
	m_picounter++;
	m_self->cancel_schedule();
	m_self->kill_children(timestamp, oproot);
	m_self->fill(timestamp);
	m_self->raise_end_event_async(timestamp, oproot);
	if(m_self->sync_node()->is_excl() && (m_self->paused() || m_self->deferred())) {
		excl* p = qualify<excl*>(m_self->sync_node());
		p->remove(m_self);
	}
	// next is postactive if its interval was not cut short (normal)
	// next is reset if the parent repeats or restarts 	(reset)
	// next is dead if the parent ends (kill)
}

///////////////////////
// postactive_state

// An element enters the postactive state when an interval ends.
// An element will remain in this state until the
// next interval begins or the parent ends.
// While in this state any fill effects are applied.

// Postactive enter / update state variables, compute next interval if restart != never
// Postactive do /	on sync update re-evaluate interval, 
//					enter active on timer event,
//					enter dead on parent end
// Postactive exit / ?

void postactive_state::enter(qtime_type timestamp) {
	report_state(timestamp);
	// m_interval = unchanged (last played interval that is now in the past);
	// m_picounter = unchanged (was incremeted by Active exit);
	// m_needs_remove = SET true or false depending on the fill attribute;
	
	restart_behavior rb = m_attrs.get_restart();
	if(rb == restart_never) return;
	
	// evaluate next interval if restart != never.
	m_played = m_interval;
	interval_type i = m_self->calc_next_interval();
	if(i.is_valid()) {	
		// Set the calc interval as current and update dependents
		m_self->schedule_interval(timestamp, i);
		// The above call may result 
		// a) to a transition to active if the interval starts at timestamp
		// b) to remain postactive waiting for the scheduled interval
	} 
	// else wait in the current state for new happenings 
}

void postactive_state::sync_update(qtime_type timestamp) {
	restart_behavior rb = m_attrs.get_restart();
	if(rb == restart_never) return;
	
	interval_type i = m_self->calc_next_interval();
	if(m_interval != m_played && i.is_valid() && i != m_interval) {
		m_self->cancel_interval(timestamp);
		m_self->schedule_interval(timestamp, i);
	} else if(m_interval != m_played && !i.is_valid()) {
		m_self->cancel_interval(timestamp);
		m_interval = m_played;
	} else if(m_interval == m_played && i.is_valid()) {
		m_self->schedule_interval(timestamp, i);
	}
}

void postactive_state::kill(qtime_type timestamp, time_node *oproot) {
	// cancels any interval and transitions to dead
	if(m_interval.is_valid() && m_interval != m_played)
		m_self->cancel_interval(timestamp);
		
	// fill remains
	m_self->set_state(ts_dead, timestamp, oproot);
}

void postactive_state::reset(qtime_type timestamp, time_node *oproot) {
	// cancels any interval and transitions to reset
	if(m_interval.is_valid() && m_interval != m_played)
		m_self->cancel_interval(timestamp);
	
	// fill goes
	m_self->remove(timestamp);	
	m_self->set_state(ts_reset, timestamp, oproot);
}	 

void postactive_state::exit(qtime_type timestamp, time_node *oproot) {
	// next is active when restart != never and next interval is within parent AD 
	// next is reset if the parent repeats or restarts 
	// next is dead if the parent ends
}

///////////////////////
// dead_state

// An element enters the dead state when its parent ends.
// While in this state any fill effects are applied.

// Dead enter / update state variables
// Dead do / nothing
// Dead exit / ?

void dead_state::enter(qtime_type timestamp) {
}

void dead_state::exit(qtime_type timestamp, time_node *oproot) {
	// next is reset if the parent restarts 
	m_self->remove(timestamp);
}


//////////////////////////
// tracing helper

const char* smil2::time_state_str(time_state_type state) {
	switch(state) {
		case ts_reset: return "reset";
		case ts_proactive: return "proactive";
		case ts_active: return "active";
		case ts_postactive: return "postactive";
		case ts_dead: return "dead";
	}
	return "unknown";
}


