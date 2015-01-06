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

#include "ambulant/smil2/time_state.h"
#include "ambulant/smil2/time_node.h"
#include "ambulant/lib/logger.h"
#include "ambulant/smil2/test_attrs.h"

//#define AM_DBG if(1)
//#define AM_DBG if(m_self->has_debug())
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
	m_active(tn->m_active),
	m_needs_remove(tn->m_needs_remove),
	m_last_cdur(tn->m_last_cdur),
	m_rad(tn->m_rad),
	m_pad(tn->m_pad),
	m_precounter(tn->m_precounter),
	m_impldur(tn->m_impldur),
	m_attrs(tn->m_attrs) {
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
	AM_DBG logger::get_logger()->debug("%s --> %s at PT:%ld, DT:%ld",
		m_self->get_sig().c_str(),
		time_state_str(this->ident()),
		timestamp.second(),
		timestamp.as_doc_time()());
}
void time_state::report_state() {
	AM_DBG logger::get_logger()->debug("%s --> %s",
		m_self->get_sig().c_str(),
		time_state_str(this->ident()));
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
	m_active = false;
	m_needs_remove = false;
	m_last_cdur = time_type::unresolved;
	m_rad = 0;
	m_pad = 0;
	m_precounter = 0;
	m_impldur = time_type::unresolved;
	m_self->clear_history();
	// Resetting the above variables is only part of the reset process.
	// See time_node::reset()
	report_state();
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
	AM_DBG logger::get_logger()->debug("%s.proactive_state::enter calc_first_interval -> %s at DT:%ld",
		m_self->get_sig().c_str(),
		::repr(i).c_str(),
		timestamp.as_doc_time()());
	if(i.is_valid()) {
		// Set the calc interval as current and update dependents
		m_self->set_interval(timestamp, i);
		// The above call may result
		// a) to a transition to active if the interval contains timestamp
		// b) to remain proactive waiting for the scheduled interval
		// c) to a jump to postactive if the interval is in the past.
	}
	// else wait in the current state for new happenings
}

void proactive_state::sync_update(qtime_type timestamp) {
	interval_type i = m_self->calc_first_interval();
	AM_DBG logger::get_logger()->debug("%s.proactive_state::sync_update %s --> %s at DT:%ld",
		m_self->get_sig().c_str(),
		::repr(m_interval).c_str(),
		::repr(i).c_str(),
		timestamp.as_doc_time()());
	if(m_interval.is_valid() && i.is_valid() && i != m_interval) {
		m_self->update_interval(timestamp, i);
	} else if(m_interval.is_valid() && !i.is_valid()) {
		m_self->cancel_interval(timestamp);
	} else if(!m_interval.is_valid() && i.is_valid()) {
		m_self->set_interval(timestamp, i);
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
	// XXX Not strictly correct for fill=transition, which now behaves identical to fill=hold
	if(m_self->up() && m_self->up()->is_seq()) {
		time_node *prev = m_self->previous();
		if(prev) {
			const time_attrs* ta = prev->get_time_attrs();
			fill_behavior fb = ta->get_fill();
			if(fb != fill_hold && fb != fill_transition)
				prev->remove(timestamp);
		}
	}
	// If we do dynamic processing of content control: see whether we
	// need to skip this node.
	const bool skip = common::preferences::get_preferences()->m_dynamic_content_control;
	if (skip && !m_self->m_in_hyperjump_path) {
		test_attrs ta(m_self->dom_node());
		if (!ta.selected()) {
			m_self->set_state(ts_postactive, timestamp, m_self);
			return;
		}
	}
	// We have to implement expr testing somewhere, and this seems like
	// a likely place. But note that I (Jack) picked this spoot without
	// thinking too hard about it, so if it turns out to be a silly place
	// there's no deeper philosophy behind it:-)
	const node *n = m_self->dom_node();
	const char *expr = n->get_attribute("expr");
	if (expr) {
		common::state_component *sc = n->get_context()->get_state();
		if (sc) {
            bool shouldplay = sc->bool_expression(expr);
            AM_DBG lib::logger::get_logger()->debug("%s::active_state::enter: expr=%d", m_self->get_sig().c_str(), (int)shouldplay);
			if (!shouldplay) {
				/* expr is false: skip the node */
				m_self->set_state(ts_postactive, timestamp, m_self);
				return;
			}
		} else {
			lib::logger::get_logger()->trace("No state engine, ignoring expr on %s", n->get_sig().c_str());
		}
	}
	// Send feedback to the upper layers about what we're doing
	m_self->node_started();

	// The timestamp in parent simple time
	// Children should convert it to their parent
    AM_DBG lib::logger::get_logger()->debug("%s::active_state::enter: resetting children", m_self->get_sig().c_str());
	m_self->reset_children(timestamp, m_self);

	// Prepare children playables without recursion
	m_self->prepare_playables();

	// Use can slip sync behavior
	// To avoid flashing use async activation
	// for audio and video only.
	// XXX: check that discrete media are local
	//if(m_self->is_cmedia())
	//	m_self->activate_async(timestamp);
	//else
	m_self->activate(timestamp);

	// The timestamp in parent simple time
	// Children should convert it to their parent
	m_self->startup_children(timestamp);

	// raise_begin_event async
	// XXX: Check excl pause
	m_self->raise_begin_event(timestamp);
}

void active_state::sync_update(qtime_type timestamp) {
	// Update end, consider restart semantics
	AM_DBG logger::get_logger()->debug("%s.active_state::sync_update() at ST:%ld PT:%ld, DT:%ld",
		m_self->get_sig().c_str(),
		timestamp.as_time_value_down_to(m_self),
		timestamp.second(),
		timestamp.as_doc_time_value());

	time_type end = m_self->calc_current_interval_end();
	if(end != m_interval.end) {
		m_self->update_interval_end(timestamp, end);
	}
	restart_behavior rb = m_attrs.get_restart();
	if(rb == restart_always
			&& !m_self->sync_node()->is_seq()
			) {
		interval_type candidate(m_interval.begin, timestamp.second);
		interval_type i = m_self->calc_next_interval(candidate);
		if(i.is_valid()) {
			AM_DBG logger::get_logger()->debug("%s restart interval %s",
				m_self->get_sig().c_str(),
				::repr(i).c_str());
			m_self->set_state(ts_postactive, timestamp, m_self);
#if 1
			// XXX Attempt by Jack to fix bug #1627916:
			// The original code here is completely different from what happens in
			// postactive/preactive. Try to run the original code by getting our
			// time_node to do sync_update recursively.
            // Note by Jack (5-Jul-2014): the fix to clear m_rad and such
            // in set_interval may obviate the needfor this hack. Unsure...h
			m_self->sync_update(timestamp);
#else
			m_self->set_begin_event_inst(timestamp.second);
			m_self->raise_update_event(timestamp);
			//m_self->sync_update(timestamp);
#endif
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
	m_self->reset_children(timestamp, oproot);

}

void active_state::exit(qtime_type timestamp, time_node *oproot) {
	m_active = false;
	m_self->fill(timestamp); // pause or stop
	m_self->kill_children(timestamp, oproot);
	AM_DBG lib::logger::get_logger()->debug("active_state::exit(%s)", m_self->get_sig().c_str());
	m_self->raise_end_event(timestamp, oproot);
	m_self->played_interval(timestamp);

	// next is postactive if its interval was not cut short (normal)
	// next is reset if the parent repeats or restarts 	(reset)
	// next is dead if the parent ends (kill)
	// Add to history and invalidate
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
	// m_needs_remove = SET true or false depending on the fill attribute;

	AM_DBG lib::logger::get_logger()->debug("postactive_state::enter(%s)", m_self->get_sig().c_str());
	if(m_self->sync_node()->is_seq()) {
		m_self->set_state(ts_dead, timestamp, m_self->sync_node());
		return;
	}

	restart_behavior rb = m_attrs.get_restart();
	if(rb == restart_never) return;

	// evaluate next interval if restart != never.
	interval_type i = m_self->calc_next_interval();
	if(i.is_valid()) {
		// Set the calc interval as current and update dependents
		m_self->set_interval(timestamp, i);
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
	if(m_interval.is_valid() && i.is_valid() && i != m_interval) {
		m_self->update_interval(timestamp, i);
	} else if(m_interval.is_valid() && !i.is_valid()) {
		m_self->cancel_interval(timestamp);
	} else if(!m_interval.is_valid() && i.is_valid()) {
		m_self->set_interval(timestamp, i);
	}
}

void postactive_state::kill(qtime_type timestamp, time_node *oproot) {
	// cancels any interval and transitions to dead
	if(m_interval.is_valid())
		m_self->cancel_interval(timestamp);

	// fill remains
	m_self->set_state(ts_dead, timestamp, oproot);
}

void postactive_state::reset(qtime_type timestamp, time_node *oproot) {
	// cancels any interval and transitions to reset
	if(m_interval.is_valid())
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
	report_state();
}

void dead_state::kill(qtime_type timestamp, time_node *oproot) {
}

void dead_state::reset(qtime_type timestamp, time_node *oproot) {
	m_self->set_state(ts_reset, timestamp, oproot);
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


