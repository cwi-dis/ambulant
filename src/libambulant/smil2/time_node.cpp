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

#include "ambulant/smil2/time_node.h"
#include "ambulant/smil2/animate_n.h"
#include "ambulant/smil2/animate_e.h"
#include "ambulant/smil2/time_calc.h"
#include <cmath>
#include <stack>
#include <set>
#include <list>

#include "ambulant/lib/logger.h"

// Use this to enable debug prints for everything:
//#define AM_DBG if(1)

// Use this to enable debug prints for all SMIL elements that have a _debug="..." attribute:
//#define AM_DBG if (has_debug())

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#include <iostream>

using namespace ambulant;
using namespace smil2;

// static (mem verifier)
int time_node::node_counter = 0;

time_node::time_node(context_type *ctx, const node *n,
	time_container_type type /* = tc_none */, bool discrete /* = false */)
:	m_context(ctx),
	m_node(n),
	m_attrs(n),
	m_type(type),
	m_discrete(discrete),
	m_timer(0),
	m_state(0),
	m_interval(interval_type::unresolved),
	m_active(false),
	m_needs_remove(false),
	m_rad(0),
	m_precounter(0),
	m_eom_flag(false),
	m_priority(0),
	m_paused(false),
	m_pad(0),
	m_paused_sync_time(0),
	m_deferred(false),
	m_ffwd_mode(false),
	m_in_hyperjump_path(false),
	m_update_event(false, qtime_type(0, 0)),
	m_transout_sr(0),
	m_begin_event_inst(time_type::unresolved),
	m_domcall_rule(0),
	m_locked(false),
	m_want_activate_events(false),
	m_want_focusin_events(false),
	m_want_focusout_events(false),
	m_want_inbounds_events(false),
	m_want_outofbounds_events(false),
	m_want_accesskey(false),
	m_impldur(time_type::unresolved),
	m_last_cdur(time_type::unresolved),
	m_logger(0),
	m_parent(0), m_next(0), m_child(0) {
	assert(type <= tc_none);
	node_counter++;
	m_logger = lib::logger::get_logger();
	m_time_calc = new time_calc(this);
	create_time_states();
	m_state = m_time_states[ts_reset];
}

time_node::~time_node() {
	node_counter--;

	// Delete the timer of this node
	delete m_timer;

	// Delete begin and end list rules
	rule_list::iterator it;
	for(it=m_begin_list.begin();it!=m_begin_list.end();it++)
		delete (*it);
	for(it=m_end_list.begin();it!=m_end_list.end();it++)
		delete (*it);
	delete m_transout_sr;

	// This node owns the lists but not
	// the sync_rules within the lists.
	dependency_map::iterator dit;
	for(dit=m_dependents.begin();dit!=m_dependents.end();dit++)
		delete (*dit).second;

	// delete this time states
	for(int i=0;i<=ts_dead;i++)
		delete m_time_states[i];
	delete m_time_calc;
	// Delete recursively this branch
	node_navigator<time_node>::delete_tree(this);
}

// A time node can be at any moment in one of the following 5 states:
// reset, proactive, active, postactive, dead
// So, a node is a FSM.
// For the current implementation the engine is
// represented by this node whereas the states
// by an instance of the time_state class.
void time_node::create_time_states() {
	m_time_states[ts_reset] = new reset_state(this);
	m_time_states[ts_proactive] = new proactive_state(this);
	m_time_states[ts_active] = new active_state(this);
	m_time_states[ts_postactive] = new postactive_state(this);
	m_time_states[ts_dead] = new dead_state(this);
}

// Helper function that collects time instances from the provided rules.
void time_node::get_instance_times(const rule_list& rules, time_mset& set) const {
	rule_list::const_iterator it;
	for(it=rules.begin();it!=rules.end();it++)
		(*it)->get_instance_times(set);
}

// Helper function that resets time instances of the provided rules.
// Resets instance times following the spec rules for a reset.
void time_node::reset(rule_list& rules, time_node *src) {
	rule_list::iterator it;
	for(it=rules.begin();it!=rules.end();it++)
		(*it)->reset(src);
}

// DOM TimeElement::startElement()
// Starts a node at t = 0.
// Currently supported only for the root.
void time_node::start() {

	if(!m_domcall_rule) {
		m_domcall_rule = new event_rule(this, tn_dom_call);
		add_begin_rule(m_domcall_rule);
	}

	qtime_type timestamp(sync_node(), 0);

	// Bring node to live
	if(!is_alive())
		set_state(ts_proactive, timestamp, this);
	// Add event instance
	m_domcall_rule->add_instance(timestamp, 0);
	exec(timestamp); // XXXJACK experimental...
}

// DOM TimeElement::stopElement()
// Currently supported only for the root.
void time_node::stop() {
	qtime_type timestamp(this, get_simple_time());
	//set_state(ts_postactive, timestamp, this);
	reset(timestamp, this);
}

// DOM TimeElement::pauseElement()
void time_node::pause() {
	// Pause the local time line
	// Pause children
	// Pause playable if a media node
}

// DOM TimeElement::resumeElement()
void time_node::resume() {
	// Resume the local time line
	// Resume children
	// Resume playable if a media node
}

// Adds a sync_rule to the begin list of this node.
// This node is not the sync base but the target (e.g. the node that will be affected).
// This is similar to how begin and end conditions are specified in the smil doc.
// This node is the owner of the sync_rule but not the source of the event.
void time_node::add_begin_rule(sync_rule *sr) {
	sr->set_target(this, rt_begin);
	time_node* tn = sr->get_syncbase();
	sync_event se = sr->get_syncbase_event();
	assert(tn!=0);
	tn->add_dependent(sr, se);
	m_begin_list.push_back(sr);
}

// Adds a sync_rule to the end list of this node.
// This node is not the sync base but the target.
void time_node::add_end_rule(sync_rule *sr) {
	sr->set_target(this, rt_end);
	time_node* tn = sr->get_syncbase();
	sync_event se = sr->get_syncbase_event();
	assert(tn!=0);
	tn->add_dependent(sr, se);
	m_end_list.push_back(sr);
}

// Sets a sync_rule for the out transition of this node.
// This node is not the sync base but the target.
void time_node::set_transout_rule(sync_rule *sr) {
	sr->set_target(this, rt_transout);
	time_node* tn = sr->get_syncbase();
	sync_event se = sr->get_syncbase_event();
	assert(tn!=0);
	tn->add_dependent(sr, se);
	m_transout_sr = sr;
}

// Adds a sync arc from this node to a target
void time_node::add_dependent(sync_rule *sr, sync_event ev) {
	AM_DBG m_logger->debug("%s.add_dependent(%s, %s)", get_sig().c_str(), sr->to_string().c_str(), sync_event_str(ev));
	dependency_map::iterator it = m_dependents.find(ev);
	rule_list *p = 0;
	if(it == m_dependents.end() || (*it).second == 0) {
		p = new rule_list();
		m_dependents[ev] = p;
	} else {
		p = (*it).second;
	}
	sr->set_syncbase(this, ev);
	p->push_back(sr);
}

// Retuns the local name of this node
std::string time_node::to_string() const {
	if(get_type() == tc_none)
		return dom_node()->get_local_name();
	return get_type_as_str();
}

std::string time_node::get_sig() const {
	std::string rv = "time_node(" + to_string() + ", ";
	const lib::node *domnode = dom_node();
	if (domnode)
		rv += domnode->get_sig();
	else
		rv += "NULL";
	rv += ")";
	return rv;
}

// Returns the implicit duration of this node.
// This is an implementation for a leaf-node.
// e.g. it queries playable for the implicit dur.
// The last definite implicit duration returned by the
// playable is stored in the variable m_impldur.
// This function is called if and only if the implicit
// duration of a node is required by the timing model.
// See time_container::get_implicit_dur()
// See seq::get_implicit_dur()
time_node::time_type
time_node::get_implicit_dur() {

	// This function is not applicable for containers.
	// Assert this usage.
	assert(!is_time_container());

	// If this is a discrete leaf node, we know the answer
	if(is_discrete()) {
		AM_DBG m_logger->debug("get_implicit_dur(%s) return 0 for discrete", get_sig().c_str());
		return time_type(0);
	}

	// Was the implicit duration calculated before?
	// If yes, avoid the overhead of querring
	if(m_impldur != time_type::unresolved) {
		AM_DBG m_logger->debug("get_implicit_dur(%s) return cached %s", get_sig().c_str(), ::repr(m_impldur).c_str());
		return m_impldur;
	}

	// Can the associated playable provide the implicit duration?
	m_impldur = get_playable_dur();

	if(m_ffwd_mode && m_impldur == time_type::unresolved) {
		AM_DBG m_logger->debug("get_implicit_dur(%s) return 1000 for ffwd_mode", get_sig().c_str());
		return 1000;
	}

	// Trace the return value of this function
	AM_DBG m_logger->debug("%s.get_implicit_dur(): %s", get_sig().c_str(), ::repr(m_impldur).c_str());

	return m_impldur;
}

void
time_node::set_ffwd_mode(bool b)
{
	AM_DBG m_logger->debug("set_ffwd_mode(%d) for %s", (int)b, get_sig().c_str());
	m_ffwd_mode = b;
	if (m_ffwd_mode && is_playable()) {
		AM_DBG m_logger->debug("set_ffwd_mode: stop_playable(%s)", get_sig().c_str());
		stop_playable();
	}
}

void
time_node::sync_playable_clock()
{
	if (!is_playable()) return;
	const lib::node *n = dom_node();
	assert(n);
	qtime_type timestamp(this, get_simple_time());
	time_type ad_offset = timestamp.second - m_interval.begin;
	// XXXJACK: does this work correctly for repeating nodes??
	m_context->seek_playable(n, time_type_to_secs(ad_offset()));
}

// This function calculates the simple duration of this node.
// See spec: "Defining the simple duration"
// The last calculated simple duration is stored in the variable m_last_cdur.
// This function will call the function "get_implicit_dur()" if and only if
// the implicit duration of a node is required by the timing model.
// Delegates the actual work to the associated time_calc instance.
time_node::time_type
time_node::calc_dur() {
	m_last_cdur = m_time_calc->calc_dur();
	return m_last_cdur;
}

// Returns true when the implicit duration is
// required by the model for timing calculations
bool time_node::needs_implicit_dur() const {
	if(is_time_container() || is_discrete())
		return false;
	if(!m_attrs.has_dur_specifier() && m_attrs.specified_end()) {
		return false;
	}
	dur_type dt = m_attrs.get_dur_type();
	if(dt == dt_unspecified || dt == dt_media)
		return true;
	return false;
}

// Calculates and returns the current inteval end.
// This uses calc_ad() function.
// Delegates the actual work to the associated time_calc instance.
time_node::time_type
time_node::calc_current_interval_end()	{
	time_mset end_list;
	get_instance_times(m_end_list, end_list);
	return m_time_calc->calc_interval_end(m_interval, end_list);
}

// Calculates the first valid interval for this node.
// Delegates the actual work to the associated time_calc instance.
// Calcs are done within the context of a parent simple dur
// A valid interval has to overlap with parent's simple duration
time_node::interval_type
time_node::calc_first_interval() {

	// Get the begin instance list
	time_mset begin_list;
	get_instance_times(m_begin_list, begin_list);
	if(m_begin_event_inst != time_type::unresolved) {
		begin_list.insert(m_begin_event_inst);
		m_begin_event_inst = time_type::unresolved;
	}

	// Get the end instance list
	time_mset end_list;
	get_instance_times(m_end_list, end_list);

	// Parent simple duration
	time_type parent_simple_dur = up()?up()->get_last_dur():time_type::indefinite;

	return m_time_calc->calc_first_interval(begin_list, end_list, parent_simple_dur);
}

// Calculates the next acceptable interval for this node.
// Delegates the actual work to the associated time_calc instance.
// Calcs are done within the context of a parent simple dur
// An valid interval has to overlap with parent's simple duration
// An interval should begin after the previous end
// A zero-dur previous interval affects calcs
time_node::interval_type
time_node::calc_next_interval(interval_type prev) {
	if(prev == interval_type::unresolved) {
		if(m_history.empty()) {
			// xxx: add warn
			return interval_type::unresolved;
		}
		prev = m_history.back();
	}

	// Get the begin instance list
	time_mset begin_list;
	get_instance_times(m_begin_list, begin_list);
	if(m_begin_event_inst != time_type::unresolved) {
		begin_list.insert(m_begin_event_inst);
		m_begin_event_inst = time_type::unresolved;
	}

	// Get the end instance list
	time_mset end_list;
	get_instance_times(m_end_list, end_list);

	// Parent simple dur
	time_type parent_simple_dur = up()?up()->get_last_dur():time_type::indefinite;

	return m_time_calc->calc_next_interval(begin_list, end_list, parent_simple_dur, prev.end,
		prev.is_zero_dur());
}

// Sets the state of this node.
// timestamp: "scheduled now" in parent simple time
// This is the state change hook for this node.
void time_node::set_state(time_state_type state, qtime_type timestamp, time_node *oproot) {
	AM_DBG m_logger->debug("set_state(%s) %s -> %s", get_sig().c_str(), time_state_str(m_state->ident()), time_state_str(state));
	if(m_state->ident() == state) return;

	// Stopgap bug fix by Jack: while seeking to a destination node,
	// that node itself may go from active to postactive to active again, without a
	// reset in between. It may still be in fill mode, though, so we should do
	// part of the reset work.
	if (m_state->ident() == ts_postactive && state == ts_active) {
		AM_DBG m_logger->debug("set_state: %s: going from ts_postactive straight to ts_active, inserting reset",
			m_node->get_sig().c_str());
		remove(timestamp);
	}
	m_state->exit(timestamp, oproot);
	m_state = m_time_states[state];
	m_state->enter(timestamp);
}

// Calls set_state() after checking for excl
void time_node::set_state_ex(time_state_type tst, qtime_type timestamp) {
	// this should be true
	assert(timestamp.first == sync_node());

	if(sync_node()->is_excl()) {
		excl *p = static_cast<excl*>(sync_node());
		if(tst == ts_active) {
			p->interrupt(this, timestamp);
		} else if(tst == ts_postactive) {
			set_state(tst, timestamp, this);
			p->on_child_normal_end(this, timestamp);
		}
		return;
	}
	set_state(tst, timestamp, this);
}


// Cancels the current interval notifyings dependents.
void time_node::cancel_interval(qtime_type timestamp) {
	AM_DBG m_logger->debug("%s.cancel_interval(): %s", get_sig().c_str(), ::repr(m_interval).c_str());
	assert(m_interval.is_valid());

	// The interval should be updated before sync_update
	// to make available the new info to induced calcs
	interval_type i = m_interval;
	m_interval = interval_type::unresolved;

	on_cancel_instance(timestamp, tn_begin, i.begin);
	on_cancel_instance(timestamp, tn_end, i.end);
}

// Updates the current interval with the one provided notifyings dependents.
void time_node::update_interval(qtime_type timestamp, const interval_type& new_interval) {
	AM_DBG m_logger->debug("%s.update_interval(): %s -> %s", get_sig().c_str(), ::repr(m_interval).c_str(), ::repr(new_interval).c_str());
	assert(m_interval.is_valid());
	assert(timestamp.first == sync_node());

	// The interval should be updated before sync_update
	// to make available the new info to induced calcs
	interval_type old = m_interval;
	m_interval = new_interval;

	if(m_interval.begin != old.begin) {
		qtime_type qt(sync_node(), m_interval.begin);
		on_update_instance(timestamp, tn_begin, m_interval.begin, old.begin);
		raise_update_event(timestamp);
	}
	if(m_interval.end != old.end) {
		time_type dt = m_interval.end - timestamp.second;
		if(dt.is_definite()) {
			// Sync node is probably interested for this event.
			sync_node()->raise_update_event(timestamp);
		}
		on_update_instance(timestamp, tn_end, m_interval.end, old.end);
	}
}

// Updates the current interval end with the value provided notifyings dependents.
void time_node::update_interval_end(qtime_type timestamp, time_type new_end) {
	AM_DBG m_logger->debug("%s.update_interval_end(): %s -> %s at PT:%ld, DT:%ld",
		get_sig().c_str(),
		::repr(m_interval.end).c_str(),
		::repr(new_end).c_str(),
		timestamp.second(),
		timestamp.as_doc_time_value());
	if(!m_interval.is_valid()) return;

	time_type old_end = m_interval.end;

	// The interval should be updated before sync_update
	// to make available the new info to induced calcs
	m_interval.end = new_end;

	on_update_instance(timestamp, tn_end, new_end, old_end);

	// Sync node is probably interested for this event.
	if(up()) up()->raise_update_event(timestamp);
}

// Sets a new interval as current, updates dependents and schedules activation.
// After this call the state of the node
// a) Remains the same (proactive or postactive) if the interval is after timestamp.
//	  In this case the interval is scheduled.
// b) Transitions to postactive if the interval is before timestamp
// c) Transitions to active if the interval contains timestamp
// See active_state::enter() for the activities executed
// when the node is activated.
// param timestamp: "now" in parent simple time
void time_node::set_interval(qtime_type timestamp, const interval_type& i) {
	AM_DBG m_logger->debug("%s.set_current_interval(): %s (DT=%ld)", get_sig().c_str(), ::repr(i).c_str(), timestamp.as_doc_time_value());
	// verify the assumptions made in the following code
	assert(timestamp.first == sync_node());
	assert(m_state->ident() == ts_proactive || m_state->ident() == ts_postactive);
	assert(is_root() || up()->m_state->ident() == ts_active);
#if 1
    // This code added by Jack, 03-Jun-2014, who is - as usual - unsure about scheduler mods.
    // The proplem to be solved is that a second run of an element with event-based
    if (m_rad() || m_pad()) {
        // This node has repeat-timing left over from a previous interval.
        // Clear it.
        m_logger->debug("%s.set_interval(%s): clearing old repeat-duration (m_rad=%ld, m_pad=%ld)", get_sig().c_str(), ::repr(i).c_str(), m_rad(), m_pad());
        m_rad = 0;
        m_pad = 0;
        m_precounter = 0;
    }
#endif
	if(!can_set_interval(timestamp, i)) {
		raise_update_event(timestamp);
		return;
	}

	// Set the interval as current.
	m_interval = i;

	// Update dependents, event if this interval will never play
	on_new_instance(timestamp, tn_begin, m_interval.begin);
	on_new_instance(timestamp, tn_end, m_interval.end);

	// Update parent to recalc end sync status
	if(sync_node()->is_par() || sync_node()->is_excl())
		sync_node()->raise_update_event(timestamp);

	// Is this a cut-off interval?
	// this can happen when proactive
	if(m_interval.before(timestamp.second)) {

		assert(m_state->ident() == ts_proactive);

		// Add to history and invalidate
		played_interval(timestamp);

		// Jump to post active
		set_state(ts_postactive, timestamp, this);

		return;
	}

	if(m_interval.after(timestamp.second)) {
		// Schedule activation:
		// activate at m_interval.begin - timestamp
		return;
	}

	// Else, the interval should be activated now
	assert(m_interval.contains(timestamp.second));
	if(deferred()) defer_interval(timestamp);
	else set_state_ex(ts_active, timestamp);
}

// Returns true when the node can establish its interval
// Fixes biased intervals
// The design should be improved so that this function always returns true.
bool time_node::can_set_interval(qtime_type timestamp, const interval_type& i) {
	if(is_root() || i.after(timestamp.second)) return true;
	if(up() && up()->is_seq()) {
		// the node should go active but the interval may be wrong/biased
		time_node *prev = previous();
		if(prev && prev->is_active()) {
			// wait
			AM_DBG m_logger->debug("time_node: %s attempt to set_current_interval() but prev active: %s (DT=%ld)", get_sig().c_str(), ::repr(i).c_str(), timestamp.as_doc_time_value());
			return false;
		}
	}
	// if an ancestor is paused or deferred return false
	std::list<const time_node*> path;
	get_path(path);
	std::list<const time_node*>::reverse_iterator it = path.rbegin();
	it++; // pass over this
	for(;it!=path.rend();it++) {
		const time_node *atn = (*it);
		if(atn->paused() || atn->deferred()) {
			AM_DBG {
				std::string astate;
				if(atn->paused()) astate = "paused";
				else if(atn->deferred()) astate = "deferred";
				m_logger->debug("time_node: %s attempt to set_current_interval() but an ancestor is %s: %s (DT=%ld)",
					get_sig().c_str(), astate.c_str(),
					::repr(i).c_str(), timestamp.as_doc_time_value());
			}
			return false;
		}
	}
	return true;
}

// Add to history and invalidate the current interval
void time_node::played_interval(qtime_type timestamp) {
	m_history.push_back(m_interval);
	q_smil_time b(sync_node(), m_interval.begin);
	q_smil_time e(sync_node(), m_interval.end);
	m_doc_history.push_back(interval_type(b.as_doc_time(), e.as_doc_time()));
	m_interval = interval_type::unresolved;
}

// Returns the first interval played by this node.
// Returns an invalid interval when this node has not played.
const time_node::interval_type& time_node::get_first_interval(bool asdoc /* = false*/) const {
	if(asdoc) {
		if(!m_doc_history.empty())
			return m_doc_history.front();
		return interval_type::unresolved;
	}
	if(!m_history.empty())
		return m_history.front();
	return m_interval;
}

// Returns the last interval associated with this (can be invalid)
const time_node::interval_type& time_node::get_last_interval() const {
	if(m_interval.is_valid())
		return m_interval;
	if(!m_history.empty())
		return m_history.back();
	return m_interval;
}

// Activates the interval of this node.
// This function is one of the activities executed when a node enters the active state.
// See active_state::enter() function for the complete list of activities.
//
// timestamp: "scheduled now" in parent simple time
void time_node::activate(qtime_type timestamp) {

	// verify the assumptions made in the following code
	assert(timestamp.first == sync_node());
	if(!m_interval.contains(timestamp.second))
		set_state(ts_postactive, timestamp, this);
	assert(timestamp.second >= 0);

	// We need to convert parent's simple time to this node's simple time.
	// For this convertion we need the dur since,
	// t_p = t_c + rad_c + begin_c	=> rad_c + t_c = t_p - begin_c
	// => t_c = rem(t_p - begin_c, dur) and rad_c = mod(t_p - begin_c, dur)*dur

	// The offset we are now within the current interval
	time_type ad_offset = timestamp.second - m_interval.begin;

	// The simple duration of this node
	time_type cdur = calc_dur();

	// Calculate the offset within the simple duration
	// that this node should start playing.
	time_type sd_offset = 0;
	if(ad_offset == 0) {
		sd_offset = 0;
	} else if(!cdur.is_definite()) {
		sd_offset = ad_offset;
	} else if(cdur == 0) {
		sd_offset = 0;
	} else {
		sd_offset = ad_offset.rem(cdur);

		// In this case we need to update the values of the repeat registers.
		// Previous values: m_rad(0), m_precounter(0)

		// The current repeat index
		m_precounter = ad_offset.mod(cdur);

		// The accumulated repeat interval.
		m_rad = m_precounter*cdur();

	}

	AM_DBG m_logger->debug("%s.start(%ld) ST:%ld, PT:%ld, DT:%ld", get_sig().c_str(),  sd_offset(), sd_offset(),
		timestamp.second(),
		timestamp.as_doc_time_value());
	// Adjust timer
	if(m_timer) {
		m_timer->set_time(ad_offset());
		m_timer->set_speed(m_attrs.get_speed());
	}

	// Start node
	if(!paused()) {
		if(is_animation()) start_animation(sd_offset);
		else if(is_statecommand()) {
			start_statecommand(sd_offset);
			// State commands finish immediately, make it so.
			if (m_state->ident() != ts_active) {
				m_logger->debug("time_node::activate: get_sig=%s m_state=%s != ts_active ! Continuing anyway..", get_sig().c_str(), time_state_str(m_state->ident()));
				return;
			}
			raise_update_event(timestamp);
			sync_node()->raise_update_event(timestamp);
		}
		else if (is_prefetch()) {
			start_prefetch(sd_offset);
			assert(m_state->ident() == ts_active);
			raise_update_event(timestamp);
			sync_node()->raise_update_event(timestamp);
		}
		else start_playable(sd_offset);
		if(m_timer) m_timer->resume();
	}
}

// Starts an animation
void time_node::start_animation(time_type offset) {
	qtime_type timestamp(this, offset);
	AM_DBG m_logger->debug("%s.start_animation(%ld) DT:%ld", get_sig().c_str(), offset(), timestamp.as_doc_time_value());
	animation_engine *ae = m_context->get_animation_engine();
	animate_node *an = (animate_node*)this;
	an->prepare_interval();
	ae->started(an);
}

// Stops an animation
void time_node::stop_animation() {
	animation_engine *ae = m_context->get_animation_engine();
	ae->stopped((animate_node*)this);
}

void time_node::start_statecommand(time_type offset) {
	qtime_type timestamp(this, offset);
	AM_DBG m_logger->debug("%s.start_statecommand(%ld) DT:%ld", get_sig().c_str(), offset(), timestamp.as_doc_time_value());
	/* XXXJACK Inline implementation should go somewhere else, at some point */
	common::state_component *sc = m_node->get_context()->get_state();
	if (!sc) {
		lib::logger::get_logger()->trace("%s: no state engine, ignoring", m_node->get_sig().c_str());
		return;
	}
	std::string tag = m_node->get_local_name();
	if (tag == "setvalue") {
		const char *ref = m_node->get_attribute("ref");
		if (!ref) {
			lib::logger::get_logger()->trace("%s: missing required ref attribute", m_node->get_sig().c_str());
			return;
		}
		const char *value = m_node->get_attribute("value");
		if (!value) {
			lib::logger::get_logger()->trace("%s: missing required value attribute", m_node->get_sig().c_str());
			return;
		}
		sc->set_value(ref, value);
	} else if (tag == "newvalue") {
		const char *ref = m_node->get_attribute("ref");
		const char *where = m_node->get_attribute("where");
		const char *name = m_node->get_attribute("name");
		if (!name) {
			lib::logger::get_logger()->trace("%s: missing required name attribute", m_node->get_sig().c_str());
			return;
		}
		const char *value = m_node->get_attribute("value");
		if (!value) {
			lib::logger::get_logger()->trace("%s: missing required value attribute", m_node->get_sig().c_str());
			return;
		}
		sc->new_value(ref, where, name, value);
	} else if (tag == "delvalue") {
		const char *ref = m_node->get_attribute("ref");
		if (!ref) {
			lib::logger::get_logger()->trace("%s: missing required ref attribute", m_node->get_sig().c_str());
			return;
		}
		sc->del_value(ref);
	} else if (tag == "send") {
		const char *submission = m_node->get_attribute("submission");
		if (!submission) {
			lib::logger::get_logger()->trace("%s: missing required submission attribute", m_node->get_sig().c_str());
			return;
		}
		const lib::node_context *ctx = m_node->get_context();
		assert(ctx);
		const lib::node *subnode = ctx->get_node(submission);
		if (!subnode || subnode->get_local_name() != "submission") {
			lib::logger::get_logger()->trace("%s: submission attribute must be ID of a submission element",
				m_node->get_sig().c_str());
			return;
		}
		sc->send(subnode);
	} else {
		assert(0);
	}
}

void time_node::start_prefetch(time_type offset) {
	if(m_ffwd_mode) {
		AM_DBG m_logger->debug("start_prefetch(%ld): ffwd skip %s", offset(), get_sig().c_str());
		return;
	}
	qtime_type timestamp(this, offset);
	AM_DBG m_logger->debug("%s.start_prefetch(%ld) DT:%ld",get_sig().c_str(), offset(), timestamp.as_doc_time_value());
	common::playable *np = create_playable();
	if(np) {
		// XXXJACK: I think offset isn't the right parameter here. The intention of the
		// preroll() first argument is that it indicates when the result of prefetching
		// is going to be needed.
		np->preroll(time_type_to_secs(offset()));
	}
}

// Returns true when this node is associated with a playable
bool time_node::is_playable() const {
	return !is_time_container() && !is_animation() && !is_statecommand() && !is_prefetch();
}

// Returns true when this node is an animation
bool time_node::is_animation() const {
	const common::schema *sch = common::schema::get_instance();
	const lib::xml_string& qn = m_node->get_local_name();
	return sch->is_animation(qn);
}

// Returns true when this node is a state command
bool time_node::is_statecommand() const {
	const common::schema *sch = common::schema::get_instance();
	const lib::xml_string& qn = m_node->get_local_name();
	return sch->is_statecommand(qn);
}

// Returns true when this node is a prefetch
bool time_node::is_prefetch() const {
	const common::schema *sch = common::schema::get_instance();
	const lib::xml_string& qn = m_node->get_local_name();
	return sch->is_prefetch(qn);
}
//////////////////////////
// Playables shell

void time_node::start_playable(time_type offset) {
	if(m_ffwd_mode) {
		AM_DBG m_logger->debug("start_playable(%ld): ffwd skip %s", offset(), get_sig().c_str());
		return;
	}
	if(!is_playable() ) {
		return;
	}
	qtime_type timestamp(this, offset);
	AM_DBG m_logger->debug("%s.start_playable(%ld) DT:%ld", get_sig().c_str(), offset(), timestamp.as_doc_time_value());
	m_eom_flag = false;
	m_saw_on_bom = false;
	m_saw_on_eom = false;
	common::playable *np = create_playable();
	if(np) np->wantclicks(m_want_activate_events);
	const lib::transition_info *trans_in = m_attrs.get_trans_in();
	if(np) {
		if(trans_in) {
			m_context->start_playable(m_node, time_type_to_secs(offset()), trans_in);
		} else {
			np->start(time_type_to_secs(offset()));
		}
	}
	if (is_link()  && m_attrs.get_actuate() == actuate_onload) {
		AM_DBG m_logger->debug("%s.start_playable: actuate_onLoad", get_sig().c_str());
		follow_link(timestamp);
	}
}

void time_node::seek_playable(time_type offset) {
	if(!is_playable() || m_ffwd_mode) return;
	AM_DBG m_logger->debug("%s.seek(%ld)", get_sig().c_str(), offset());
	m_context->seek_playable(m_node, time_type_to_secs(offset()));
}

void time_node::pause_playable(common::pause_display d) {
	if(!is_playable() || m_ffwd_mode) return;
	AM_DBG m_logger->debug("%s.pause()", get_sig().c_str());
	m_context->pause_playable(m_node, d);
}

void time_node::resume_playable() {
	if(!is_playable() || m_ffwd_mode) return;
	AM_DBG m_logger->debug("%s.resume()", get_sig().c_str());
	m_context->resume_playable(m_node);
}

void time_node::stop_playable() {
	if(!is_playable() && !is_prefetch()) return;
	if(!m_needs_remove) return;
	m_eom_flag = true;
	AM_DBG m_logger->debug("%s%.stop()", get_sig().c_str());
	m_context->stop_playable(m_node);
}

void time_node::repeat_playable() {
	if(!is_playable() || m_ffwd_mode) return;
	AM_DBG m_logger->debug("%.repeat()",get_sig().c_str());
	stop_playable();
	m_context->start_playable(m_node, 0);
}

common::playable *time_node::create_playable() {
	assert(is_playable()||is_prefetch());
	if(m_ffwd_mode) return 0;
	AM_DBG m_logger->debug("%s.create()", get_sig().c_str());
	return m_context->create_playable(m_node);
}

time_node::time_type time_node::get_playable_dur() {
	common::duration dur_pair = m_context->get_dur(m_node);
	if(dur_pair.first && dur_pair.second>0)
		return secs_to_time_type(dur_pair.second)();
	return time_type::unresolved;
}

// Prepare children playables without recursion
void time_node::prepare_playables() {
#ifdef WITH_AGGRESSIVE_PRELOADING
	// Note by Jack: this code is disabled. It has some serious
	// drawbacks, such as starting all playables for children of a <seq>, which
	// could (in some documents) cause an incredible number of playables to
	// be created long before they're actually needed. We need to come up
	// with a better preloading scheme at some point.
	if(m_ffwd_mode) return;
	std::list<time_node*> children;
	get_children(children);
	std::list<time_node*>::iterator it;
	for(it = children.begin(); it != children.end(); it++) {
		if((*it)->is_playable()) (*it)->create_playable();
	}
#endif
}

//////////////////////////
// Driver shell

void time_node::get_pending_events(std::map<time_type, std::list<time_node*> >& events) {
	if(!is_alive() || paused() || deferred()) return;
	if(m_interval.is_valid()) {
		if(!is_active()) {
			// If we are not active we schedule our own begin
			qtime_type timestamp(sync_node(), m_interval.begin);
			time_type doctime = timestamp.to_doc();
			AM_DBG m_logger->debug("get_pending_events(0x%x %s): schedule begin for %d (selftime %d)", this, get_sig().c_str(), doctime(), m_interval.begin());

			events[doctime].push_back(this);
		} else if(m_interval.end.is_definite()) {
			// If we are active and our end is known we schedule our own end
			qtime_type timestamp(sync_node(), get_interval_end());
			time_type doctime = timestamp.to_doc();
			AM_DBG m_logger->debug("get_pending_events(0x%x %s): schedule end for %d", this, get_sig().c_str(), doctime());
			events[doctime].push_back(this);

			bool repeats = m_attrs.specified_rdur() || m_attrs.specified_rcount();
			repeats = repeats && (m_last_cdur.is_definite() && m_last_cdur() != 0);
			if(repeats) {
				// And we also schedule our repeat, if applicable
				qtime_type ts(this, m_last_cdur);
				time_type dt = ts.to_doc();
				AM_DBG m_logger->debug("get_pending_events(0x%x %s): schedule repeat for %d", this, get_sig().c_str(), dt());
				events[dt].push_back(this);
			}
		}
	}

	if(m_update_event.first) {
		// Jack thinks (but not sure:-) this one has to do with multiple
		// begin/end times.
		qtime_type timestamp = m_update_event.second;
		time_type doctime = timestamp.to_doc();
		AM_DBG m_logger->debug("get_pending_events(0x%x %s): schedule update_event for %d", this, get_sig().c_str(), doctime());
		events[doctime].push_back(this);
	}

	if(m_transout_sr && !m_ffwd_mode) {
		// We schedule the start of our outtransition, if applicable
		time_mset set;
		m_transout_sr->get_instance_times(set);
		if(!set.empty()) {
			qtime_type timestamp(sync_node(), *set.begin());
			time_type doctime = timestamp.to_doc();
			AM_DBG m_logger->debug("get_pending_events(0x%x %s): schedule transout for %d", this, get_sig().c_str(), doctime());
			events[doctime].push_back(this);
		}
	}

	std::list<time_node*> children;
	get_children(children);
	std::list<time_node*>::iterator it;
	for(it = children.begin(); it != children.end(); it++) {
		// Finally we recursively get all events for our childen.
		(*it)->get_pending_events(events);
	}
}

void time_node::exec(qtime_type timestamp) {
	AM_DBG m_logger->debug("time_node::exec(%ld) for %s ffwd %d is_alive()=%d is_active()=%d", timestamp.second(), get_sig().c_str(), (int)m_ffwd_mode, is_alive(), is_active());
	if(!is_alive()) {
		// check for transOut
		return;
	}

	if(m_update_event.first) {
		AM_DBG m_logger->debug("timenode::exec(%ld): calling %s.sync_update(%ld)", timestamp.second(), get_sig().c_str(), m_update_event.second.second());
		sync_update(m_update_event.second);
		m_update_event.first = false;
	}

	timestamp.to_node(sync_node());
	assert(timestamp.first == sync_node());

	if(!is_active()) {
		// in this state, activation is the only interesting activity
		AM_DBG m_logger->debug("time_node::exec(%ld) for %s m_interval=(%ld,%ld) deferred=%d", timestamp.second(), get_sig().c_str(), m_interval.begin(), m_interval.end(), deferred());
		if(begin_cond(timestamp)) {
			if(deferred()) defer_interval(timestamp);
			else set_state_ex(ts_active, timestamp);
		}
		return;
	}
	// The following code applies to active nodes
	assert(is_active());

	if(m_transout_sr) {
		// timestamp.second >= m_interval.end
		time_mset set;
		m_transout_sr->get_instance_times(set);
		if(!set.empty()) {
			qtime_type ts(sync_node(), *set.begin());
			if(timestamp.second >= ts.second) {
				// start trasnition
				AM_DBG m_logger->debug("%s.start_transition() at %ld (end:%ld)",
					get_sig().c_str(),
					ts.second(),  m_interval.end());
				const lib::transition_info *trans_out = m_attrs.get_trans_out();
				m_context->start_transition(m_node, trans_out, false);
				m_transout_sr->reset(0);
			}
		}
	}

	// Check for the EOI event
	if(end_cond(timestamp)) {
		// This node should go postactive
		time_type reftime;
		if(m_interval.end.is_definite()) reftime = m_interval.end;
		else reftime = timestamp.second;
		qtime_type qt(sync_node(), reftime);
		// Fix by Jack (who is, as usual, unsure whether it's actually correct:-):
		// We can now set the last_cdur, and we may need to schedule a repeat. Otherwise
		// we go to postactive.
		m_last_cdur = timestamp.as_time_down_to(this);
		if (!on_eosd(timestamp))
			set_state_ex(ts_postactive, qt);
		return;
	}
#if 1 // Quick fix for: #2663294
	// Note: the following code is probably a stop-gap.
	// The original code (which didn't look at repeats) was added
	// because repeating nodes with indefinite duration didn't
	// repeat correctly. However, the fix of setting m_last_cdur
	// broke hyperlinks (for example in Flashlight_desktop.smil).
	// The solution of only setting m_last_cdur when in a repeating node
	// feels more like a hack than a real fix...
	bool repeats = m_attrs.specified_rdur() || m_attrs.specified_rcount();
	if (repeats && !m_last_cdur.is_definite() && m_eom_flag) {
		time_type cdur = timestamp.as_time_down_to(this);
		m_last_cdur = cdur;
	}
#endif

	// Check for the EOSD event
	AM_DBG m_logger->debug("%s checking for end-of-sd (cdur=%ld)", get_sig().c_str(), m_last_cdur());
	if(m_last_cdur.is_definite() && m_last_cdur() != 0 && timestamp.as_time_value_down_to(this) >= m_last_cdur())
		on_eosd(timestamp);
}

// Returns true when the end conditions of this node evaluate to true
// e.g. the node should be deactivated
// Assumes the node is active
bool time_node::end_cond(qtime_type timestamp) {
	assert(timestamp.first == sync_node());
	assert(is_active());
	// This expression will be true for containers that have and endsync and that endsync is true.
	bool ec = end_sync_cond_applicable() && end_sync_cond();
	// This expression will be true for nodes whose specified duration (dur or end) has passed.
	bool tc = !end_sync_cond_applicable() && timestamp.second >= get_interval_end();

	if(is_time_container() && (ec || tc)) {
		AM_DBG m_logger->debug("%s.end_cond() true [%s]", get_sig().c_str(), (ec?"end_sync_cond":"interval_end"));
	}

	// The "tc" condition is not sufficient when needs_implicit_dur()is true
	// for example:
	// a) a video has returned its implicit dur
	// b) calcs have been done based on this value
	// c) m_interval.end may have assumed this vale
	// d) the interval has not been updated yet
	// e) due to not controled delays the video is still playing

	bool specified_dur = m_attrs.specified_dur() || m_attrs.specified_rdur();
	if (!specified_dur && (is_statecommand() || is_prefetch())) tc = true;
	if(is_cmedia() && !is_animation()
			&& !is_statecommand()
			&& !is_prefetch()
			&& tc && !specified_dur && m_time_calc->uses_dur()) {
		if(m_context->wait_for_eom() && !m_eom_flag) {
			tc = false;
			AM_DBG m_logger->debug("%s.end_cond() waiting media end",get_sig().c_str());
		}
	}

	return ec || tc;
}

// Returns true when the begin conditions of this node evaluate to true
// e.g. the node should be activated
bool time_node::begin_cond(qtime_type timestamp) {
	return m_interval.is_valid() && m_interval.contains(timestamp.second);
}

//////////////////////////
// Notifications

// The clock used by the argument timestamp can be any
// Convert it if possible to this sync_node clock time
void time_node::sync_update(qtime_type timestamp) {
	time_type pt = timestamp.as_node_time(sync_node());
	if(pt.is_resolved()) {
		timestamp = qtime_type(sync_node(), pt);
		m_state->sync_update(timestamp);
	}
}

// Called on the end of simple duration event
bool time_node::on_eosd(qtime_type timestamp) {
	AM_DBG m_logger->debug("%s.on_eosd() ST:%ld, PT:%ld, DT:%ld (sdur=%ld)", 
        get_sig().c_str(),
		timestamp.as_time_value_down_to(this),
		timestamp.second(),
		timestamp.as_doc_time_value(),
		m_last_cdur()
		);

	// update repeat registers
	m_rad += m_last_cdur();
	m_precounter++;

	// Should this node repeat?
	if(m_attrs.specified_rdur()) {
		time_type rdur = m_attrs.get_rdur();
		if(rdur == time_type::indefinite || rdur > m_rad) {
			repeat(timestamp);
			return true;
		}
	}

	if(m_attrs.specified_rcount()) {
		double rcount = m_attrs.get_rcount();
		if(m_attrs.is_rcount_indefinite() || rcount > double(m_precounter)) {
			repeat(timestamp);
			return true;
		}
	}
	return false;
}

// Begin of media notification
// Currently this notification is not used.
// Could be used to define the slip sync offset.
void time_node::on_bom(qtime_type timestamp) {
	AM_DBG if (m_saw_on_bom)
		m_logger->debug("time_node::on_bom: renderer emitted second started() callback for %s", get_sig().c_str());
	AM_DBG if (m_saw_on_eom)
		m_logger->debug("time_node::on_bom: renderer emitted started() callback after stopped() callback for %s", get_sig().c_str());
	if (m_saw_on_bom || m_saw_on_eom)
		return;
	m_saw_on_bom = true;

	m_eom_flag = false;
	if(!is_discrete()) {
		qtime_type pt = timestamp.as_qtime_down_to(sync_node());
		qtime_type st = pt.as_qtime_down_to(this);
		AM_DBG m_logger->debug("%s.on_bom() ST:%ld, PT:%ld, DT:%ld",
			get_sig().c_str(),
			st.second(),
			pt.second(),
			timestamp.second());
	}
	if(m_timer) m_timer->resume();
}

// Notification from the playable that has paused for fetching bits
void time_node::on_pom(qtime_type timestamp) {
	if(m_timer) m_timer->pause();
}

// Notification from the playable that has resumed playback
void time_node::on_rom(qtime_type timestamp) {
	if(m_timer) m_timer->resume();
}

// End of media notification
// This notification is taken into account when this node is still active
// and the implicit duration is involved in timing calculations.
void time_node::on_eom(qtime_type timestamp) {
	AM_DBG m_logger->debug("%s.on_eom()", get_sig().c_str());
	if (!m_saw_on_bom) {
		AM_DBG m_logger->debug("time_node::on_eom: simulating on_bom() which has not occurred yet for %s", get_sig().c_str());
		on_bom(timestamp);
	}
	if (m_saw_on_eom)
		AM_DBG m_logger->debug("time_node::on_eom: renderer emitted second stopped() callback for %s", get_sig().c_str());
	m_saw_on_eom = true;

	m_eom_flag = true;
	if(is_playable() && !is_discrete()) {
		if(m_impldur == time_type::unresolved) {
			time_type pt = timestamp.as_node_time(sync_node());
			m_impldur = pt - m_interval.begin();
		}
		if (m_state->ident() == ts_active) {
			AM_DBG m_logger->debug("time_node::on_eom: raise update event for active %s", get_sig().c_str());
			raise_update_event(timestamp);
			sync_node()->raise_update_event(timestamp);
		}
		qtime_type pt = timestamp.as_qtime_down_to(sync_node());
		qtime_type st = pt.as_qtime_down_to(this);
		AM_DBG m_logger->debug("%s.on_eom() ST:%ld, PT:%ld, DT:%ld",
			get_sig().c_str(),
			st.second(),
			pt.second(),
			timestamp.second());
	}
}

// Return true if we want on on_eom callback.
// This is a hack, really. The problem it tries to solve is that sometimes the
// on_eom notification comes in late (because it goes through the event processor)
// and by the time we get to on_eom we've already started with a next iteration.
// The on_eom callback will then erronuously terminte the new iteration.
// But: this solution is a hack, it would be much better if we (a) could make sure
// this situation could never occur or (b) could detect this situation in on_eom().
bool time_node::want_on_eom()
{
#if 1
	return true;
#else
	return is_active() && !m_eom_flag;
#endif
}

// End of transition
// This notification is sent when a transition ends. It is sent to
// all nodes that overlap the transition that just finished.
void time_node::on_transitioned(qtime_type timestamp) {
	AM_DBG m_logger->debug("%s.on_transitioned() DT:%ld",
		get_sig().c_str(),
		timestamp.second());
	// If this node is not in fill=fill_transition state we do nothing
	if (m_active || !m_needs_remove) return;
	const time_attrs* ta = get_time_attrs();
	fill_behavior fb = ta->get_fill();
	if (fb == fill_transition)
		remove(timestamp);

}

//////////////////////////
// Node activities (see also activate())

// The following function is called when the node should repeat.
// It is responsible to execute the repeat actions for this node.
void time_node::repeat(qtime_type timestamp) {
	AM_DBG m_logger->debug("%s.repeat() ST:%ld, PT:%ld, DT:%ld",
        get_sig().c_str(),
		timestamp.as_time_value_down_to(this),
		timestamp.second(),
		timestamp.as_doc_time_value());

	// raise_repeat_event async
	raise_repeat_event(timestamp);

	if(down()) {
		// The reset_children op ends all active children
		// In consequence for excl empties its pause queue
		reset_children(timestamp, this);
		startup_children(timestamp);
	}

	if(!is_time_container()) {
		repeat_playable();
	}
}

// Pauses this node.
// Excl element handling.
void time_node::pause(qtime_type timestamp, pause_display d) {
	if(!is_active()) return;
	time_type self_simple_time = timestamp.as_time_down_to(this);
	qtime_type qt(this, self_simple_time);

	if(is_playable()) pause_playable(d);
	if(m_timer) m_timer->pause();

	m_paused_sync_time = timestamp.as_time_down_to(sync_node());

	// could deduce this from the fact that the node
	// is active and the timer is nor running
	set_paused(true);

	// Recalculate the interval of this node as if its dur was indefinite
	// The interval semantics are applicable when paused
	interval_type i1 = m_interval;
	m_time_calc->set_paused_mode(true);
	time_type iend = calc_current_interval_end();
	if(iend != m_interval.end) {
		update_interval_end(timestamp, iend);
	}

	// Report changed interval
	AM_DBG m_logger->debug("%s.pause(): %s -> %s at %ld",
		get_sig().c_str(),
		::repr(i1).c_str(), ::repr(m_interval).c_str(), self_simple_time());

	// Pause playable active children
	time_node::iterator nit;
	time_node::iterator end_nit = end();
	for(nit=begin(); nit != end_nit; nit++) {
		if(!(*nit).first && (*nit).second->is_playable() && (*nit).second->is_active())
			(*nit).second->pause_playable(d);
	}
}

// Resumes this node.
// Excl element handling.
void time_node::resume(qtime_type timestamp) {
	if(!is_active()) {
		// gone inactive while paused
		AM_DBG m_logger->debug("%s.resume(): gone inactive while paused",get_sig().c_str());
		return;
	}

	m_time_calc->set_paused_mode(false);
	set_paused(false);
	AM_DBG m_logger->debug("%s.resume()",get_sig().c_str());

	time_type self_simple_time = timestamp.as_time_down_to(this);
	qtime_type qt(this, self_simple_time);

	// re-establish sync
	time_type iend = calc_current_interval_end();
	time_type resumed_sync_time = timestamp.as_time_down_to(sync_node());
	time_type pauseoffset = resumed_sync_time - m_paused_sync_time;

	m_pad += pauseoffset;
	if(iend != m_interval.end) {
		update_interval_end(timestamp, iend);
	}

	AM_DBG m_logger->debug("%s.resume(): %s",
        get_sig().c_str(),
		::repr(m_interval).c_str());

	if(is_playable()) resume_playable();
	if(m_timer) m_timer->resume();

	// Resume playable active children
	time_node::iterator nit;
	time_node::iterator end_nit = end();
	for(nit=begin(); nit != end_nit; nit++) {
		if(!(*nit).first && (*nit).second->is_playable() && (*nit).second->is_active()) {
			(*nit).second->resume_playable();
		}
	}
}

// Defers the interval of this node.
// Excl element handling.
void time_node::defer_interval(qtime_type timestamp) {
	// Mark this node as deferred.
	set_deferred(true);

	// Cancel notifications
	interval_type i = m_interval;
	if(!i.is_valid()) return;

	m_interval = interval_type::unresolved;
	on_cancel_instance(timestamp, tn_begin, i.begin);
	on_cancel_instance(timestamp, tn_end, i.end);

	// Remember interval
	m_interval = i;
}

// Excl element handling.
// When an element is deferred, the begin time is deferred as well
void time_node::set_deferred_interval(qtime_type timestamp) {
	if(!deferred()) return;

	// Remove deferred marker
	set_deferred(false);

	if(!m_interval.is_valid()) return;

	// Translate the defered interval to timestamp
	interval_type i = m_interval;
	i.translate(timestamp.second-m_interval.begin);

	// Schedule interval
	set_interval(timestamp, i);
}

// This function is called always when a node exits the active state.
// Applies the appropriate fill behavior for this node.
//
// For containers the current implementation calls first kill_children().
// Therefore this function will be called first for the children and
// then for their parent.
//
// Pause or stop peer playable based on the fill semantics.
// Results to peer.pause() (m_needs_remove = true) or peer.stop() (m_needs_remove = true).
//
void time_node::fill(qtime_type timestamp) {
	// Does this node have shown anything?
	if(!m_needs_remove) return;

	fill_behavior fb = m_attrs.get_fill();
//	fill_behavior pfb = sync_node()->get_time_attrs()->get_fill();

	bool keep = (fb != fill_remove);

	if(keep) {
		// this node should be freezed
		AM_DBG m_logger->debug("%s.pause() ST:%ld, PT:%ld, DT:%ld", get_sig().c_str(),
			timestamp.as_time_value_down_to(this), timestamp.second(),
			timestamp.as_doc_time_value());
        m_context->node_filled(m_node);
		if(down()) {
			std::list<time_node*> cl;
			get_children(cl);
			std::list<time_node*>::iterator it;
			time_type self_simple_time = timestamp.as_time_down_to(this);
			qtime_type qt(this, self_simple_time);
			for(it = cl.begin(); it != cl.end(); it++)
				(*it)->fill(qt);
		}
		if (fb != fill_continue && is_playable()) {
			pause_playable();
		} else {
			// Instead of pausing the playable, we should continue it for some short period of time.
			// Here, I just print some message and actual action needed to be inserted later after I
			// figure out how to do it.
			if (m_node->get_attribute("src")) {
				AM_DBG m_logger->debug("%s.continue() ST:%ld, PT:%ld, DT:%ld", get_sig().c_str(), timestamp.as_time_value_down_to(this), timestamp.second(), timestamp.as_doc_time_value());
			}
		}
		if(m_timer) {
			m_timer->pause();
			m_timer->set_time(m_interval.end());
		}
	} else {
		// this node should be removed
		remove(timestamp);
	}
}

// Removes any fill effects
// This is called always by reset.
// This maybe called when the element is deactivated
// or when the next is activated.
void time_node::remove(qtime_type timestamp) {
	if(!m_needs_remove) return;
	AM_DBG m_logger->debug("%s.stop() ST:%ld, PT:%ld, DT:%ld",
        get_sig().c_str(),
		timestamp.as_time_value_down_to(this),
		timestamp.second(),
		timestamp.as_doc_time_value());
	if(down()) {
		// Is this correct for a container?
		std::list<time_node*> children;
		get_children(children);
		std::list<time_node*>::iterator it;
		time_type self_simple_time = timestamp.as_time_down_to(this);
		qtime_type qt(this, self_simple_time);
		for(it = children.begin(); it != children.end(); it++)
			(*it)->remove(qt);
	}
	if(is_animation()) stop_animation();
	else if (is_prefetch()) stop_playable();
	else if(is_playable()) stop_playable();
	/* else nothing to do for statecommands */
	if(m_timer) m_timer->stop();
	m_context->node_stopped(m_node);
    m_needs_remove = false;
}

///////////////////////////////
// Dependents updates

void time_node::on_new_instance(qtime_type timestamp, sync_event ev, time_type instance, time_node *filter) {
	dependency_map::iterator dit = m_dependents.find(ev);
	if(dit != m_dependents.end() && (*dit).second != 0) {
		rule_list *p = (*dit).second;
		for(rule_list::iterator it=p->begin();it!=p->end();it++) {
			time_node* owner = (*it)->get_target();
			if(!filter || !nnhelper::is_descendent(owner, filter))
				(*it)->new_instance(timestamp, instance);
		}
	}
}

void time_node::on_update_instance(qtime_type timestamp, sync_event ev, time_type instance, time_type old_instance, time_node *filter) {
	dependency_map::iterator dit = m_dependents.find(ev);
	if(dit != m_dependents.end() && (*dit).second != 0) {
		rule_list *p = (*dit).second;
		for(rule_list::iterator it=p->begin();it!=p->end();it++) {
			time_node* owner = (*it)->get_target();
			if(!filter || !nnhelper::is_descendent(owner, filter))
				(*it)->update_instance(timestamp, instance, old_instance);
		}
	}
}

void time_node::on_cancel_instance(qtime_type timestamp, sync_event ev, time_type instance, time_node *filter) {
	dependency_map::iterator dit = m_dependents.find(ev);
	if(dit != m_dependents.end() && (*dit).second != 0) {
		rule_list *p = (*dit).second;
		for(rule_list::iterator it=p->begin();it!=p->end();it++) {
			time_node* owner = (*it)->get_target();
			if(!filter || !nnhelper::is_descendent(owner, filter))
				(*it)->cancel_instance(timestamp, instance);
		}
	}
}

// Update dependents for an event instance
// Asserts that the same event is not used to update the same element twice.
void time_node::on_add_instance(qtime_type timestamp, smil2::sync_event ev,
	time_node::time_type instance, int data, time_node *filter) {
	dependency_map::iterator dit = m_dependents.find(ev);
	if(dit == m_dependents.end() || (*dit).second == 0) {
		// no dependents
		return;
	}

	// List of rules to update
	rule_list *p = (*dit).second;

	// Set of dependents
	std::set<time_node*> dset;

	// 1. add event to not active
	// 1.1 begin
	rule_list::iterator it;
	for(it=p->begin();it!=p->end();it++) {
		time_node* owner = (*it)->get_target();
		AM_DBG m_logger->debug("%s.on_add_instance() --> %s",
			get_sig().c_str(),
			owner->get_sig().c_str());
		rule_type rt = (*it)->get_target_attr();
		if(!owner->is_active() && rt == rt_begin && dset.find(owner) == dset.end()) {
			if(!filter || !nnhelper::is_descendent(owner, filter)) {
				(*it)->add_instance(timestamp, instance, data);
				dset.insert(owner);
			}
		}
	}
	// 1.2 end
	for(it=p->begin();it!=p->end();it++) {
		time_node* owner = (*it)->get_target();
		rule_type rt = (*it)->get_target_attr();
		if(!owner->is_active() && rt == rt_end && dset.find(owner) == dset.end()) {
			if(!filter || !nnhelper::is_descendent(owner, filter)) {
				(*it)->add_instance(timestamp, instance, data);
				dset.insert(owner);
			}
		}
	}

	// 2. add event to active
	// 2.1 end
	for(it=p->begin();it!=p->end();it++) {
		time_node* owner = (*it)->get_target();
		rule_type rt = (*it)->get_target_attr();
		if(owner->is_active() && rt == rt_end && dset.find(owner) == dset.end()) {
			if(!filter || !nnhelper::is_descendent(owner, filter)) {
				(*it)->add_instance(timestamp, instance, data);
				dset.insert(owner);
			}
		}
	}
	// 2.2 begin
	for(it=p->begin();it!=p->end();it++) {
		time_node* owner = (*it)->get_target();
		rule_type rt = (*it)->get_target_attr();
		if(owner->is_active() && rt == rt_begin && dset.find(owner) == dset.end()) {
			if(!filter || !nnhelper::is_descendent(owner, filter)) {
				(*it)->add_instance(timestamp, instance, data);
				dset.insert(owner);
			}
		}
	}
}

// XXXJACK I *know* this can be done with templates, but how: this method is
// 100% identical to the previous one.
// Update dependents for an event instance
// Asserts that the same event is not used to update the same element twice.
void time_node::on_add_instance(qtime_type timestamp, smil2::sync_event ev,
	time_node::time_type instance, std::string data, time_node *filter) {
	dependency_map::iterator dit = m_dependents.find(ev);
	if(dit == m_dependents.end() || (*dit).second == 0) {
		AM_DBG m_logger->debug("No dependents for on_add_instance event on 0x%x", (void*)this);
		// no dependents
		return;
	}

	// List of rules to update
	rule_list *p = (*dit).second;

	// Set of dependents
	std::set<time_node*> dset;

	// 1. add event to not active
	// 1.1 begin
	rule_list::iterator it;
	for(it=p->begin();it!=p->end();it++) {
		time_node* owner = (*it)->get_target();
		AM_DBG m_logger->debug("%s.on_add_instance() --> %s",
			get_sig().c_str(),
            owner->get_sig().c_str());
		rule_type rt = (*it)->get_target_attr();
		if(!owner->is_active() && rt == rt_begin && dset.find(owner) == dset.end()) {
			if(!filter || !nnhelper::is_descendent(owner, filter)) {
				(*it)->add_instance(timestamp, instance, data);
				dset.insert(owner);
			}
		}
	}
	// 1.2 end
	for(it=p->begin();it!=p->end();it++) {
		time_node* owner = (*it)->get_target();
		rule_type rt = (*it)->get_target_attr();
		if(!owner->is_active() && rt == rt_end && dset.find(owner) == dset.end()) {
			if(!filter || !nnhelper::is_descendent(owner, filter)) {
				(*it)->add_instance(timestamp, instance, data);
				dset.insert(owner);
			}
		}
	}

	// 2. add event to active
	// 2.1 end
	for(it=p->begin();it!=p->end();it++) {
		time_node* owner = (*it)->get_target();
		rule_type rt = (*it)->get_target_attr();
		if(owner->is_active() && rt == rt_end && dset.find(owner) == dset.end()) {
			if(!filter || !nnhelper::is_descendent(owner, filter)) {
				(*it)->add_instance(timestamp, instance, data);
				dset.insert(owner);
			}
		}
	}
	// 2.2 begin
	for(it=p->begin();it!=p->end();it++) {
		time_node* owner = (*it)->get_target();
		rule_type rt = (*it)->get_target_attr();
		if(owner->is_active() && rt == rt_begin && dset.find(owner) == dset.end()) {
			if(!filter || !nnhelper::is_descendent(owner, filter)) {
				(*it)->add_instance(timestamp, instance, data);
				dset.insert(owner);
			}
		}
	}
}

////////////////////
// Raising events:
// beginEvent, repeat event, endEvent

// Called when this node transitions to active.
// (active_state::enter() activity)
// Containers have to reset children, bring them to life
// and notify dependents.
// Leaf nodes have to notify dependents and start their peer playable.
void time_node::raise_begin_event(qtime_type timestamp) {
	AM_DBG m_logger->debug("%s.raise_begin_event() ST:%ld, PT:%ld, DT:%ld",
        get_sig().c_str(),
		timestamp.as_time_value_down_to(this),
		timestamp.second(),
		timestamp.as_doc_time_value());
	assert(timestamp.first == sync_node());
	on_add_instance(timestamp, tn_begin_event, timestamp.second);

	// Simulate a timegraph-sampling implementation
	// For a time container we know that more info will be available
	// immediately after it starts.
	if(down() && m_interval.end == time_type::indefinite)
		raise_update_event(timestamp);
}

// Send feedback to the upper layers about what we're doing
void time_node::node_started()
{
	if(is_root()) m_context->started_playback();
	if(!m_ffwd_mode) m_context->node_started(m_node);
}

// Called when this node repeats.
// Containers have to reset children, bring them to life
// and notify dependents.
// Leaf nodes have to notify dependents and start their peer playable at 0.
// timestamp is parent's simple time when this event occurs.
void time_node::raise_repeat_event(qtime_type timestamp) {
	AM_DBG m_logger->debug("%s.raise_repeat_event(%d) ST:%ld, PT:%ld, DT:%ld",
        get_sig().c_str(),
        m_precounter,
		timestamp.as_time_value_down_to(this),
		timestamp.second(),
		timestamp.as_doc_time_value());
	assert(timestamp.first == sync_node());
	on_add_instance(timestamp, tn_repeat_event, timestamp.second, (int)m_precounter);
}

// Called when this node exits active.
// On end, containers have to kill their children
// and notify dependents.
// Leaf nodes have to notify dependents and pause playable
void time_node::raise_end_event(qtime_type timestamp, time_node *oproot) {
	AM_DBG m_logger->debug("%s.raise_end_event() ST:%ld, PT:%ld, DT:%ld",
        get_sig().c_str(),
		timestamp.as_time_value_down_to(this),
		timestamp.second(),
		timestamp.as_doc_time_value());
	if(timestamp.first != sync_node()) {
		timestamp.to_node(sync_node());
	}
	if(m_interval.end != timestamp.second) {
		on_update_instance(timestamp, tn_end, timestamp.second, m_interval.end, oproot);
		m_interval.end = timestamp.second;
	}
	on_add_instance(timestamp, tn_end_event, timestamp.second, 0, oproot);

	if(sync_node()->is_excl() && (paused() || deferred())) {
		excl* p = qualify<excl*>(sync_node());
		p->remove(this);
		set_paused(false);
		set_deferred(false);
		m_time_calc->set_paused_mode(false);
	}

	// Check parent end_sync conditions
	// call schedule_sync_update(timestamp) if needed
	time_node *p = up();
	if(p && (p->is_par() || p->is_excl() || (p->is_seq() && !next())))
		p->raise_update_event(timestamp);
	if(p && p->is_seq()) {
		time_node *n = next();
		if(n) n->raise_update_event(timestamp);
	}

	if(is_root()) 
        m_context->done_playback();
}

void time_node::raise_activate_event(qtime_type timestamp) {
	timestamp.to_descendent(sync_node());
	AM_DBG m_logger->debug("%s.raise_activate_event() ST:%ld, PT:%ld, DT:%ld",
        get_sig().c_str(),
		timestamp.as_time_value_down_to(this),
		timestamp.second(),
		timestamp.as_doc_time_value());
	on_add_instance(timestamp, tn_activate_event, timestamp.second);
	if(is_link()) {
		follow_link(timestamp);
	}
}

void time_node::raise_inbounds_event(qtime_type timestamp) {
	timestamp.to_descendent(sync_node());
	AM_DBG m_logger->debug("%s.raise_inbounds_event() ST:%ld, PT:%ld, DT:%ld",
        get_sig().c_str(),
		timestamp.as_time_value_down_to(this),
		timestamp.second(),
		timestamp.as_doc_time_value());
	on_add_instance(timestamp, tn_inbounds_event, timestamp.second);
}

void time_node::raise_outofbounds_event(qtime_type timestamp) {
	timestamp.to_descendent(sync_node());
	AM_DBG m_logger->debug("%s.raise_outofbounds_event() ST:%ld, PT:%ld, DT:%ld",
        get_sig().c_str(),
		timestamp.as_time_value_down_to(this),
		timestamp.second(),
		timestamp.as_doc_time_value());
	on_add_instance(timestamp, tn_outofbounds_event, timestamp.second);
}

void time_node::raise_focusin_event(qtime_type timestamp) {
	timestamp.to_descendent(sync_node());
	AM_DBG m_logger->debug("%s.raise_focusin_event() ST:%ld, PT:%ld, DT:%ld",
        get_sig().c_str(),
		timestamp.as_time_value_down_to(this),
		timestamp.second(),
		timestamp.as_doc_time_value());
	on_add_instance(timestamp, tn_focusin_event, timestamp.second);
}

void time_node::raise_focusout_event(qtime_type timestamp) {
	timestamp.to_descendent(sync_node());
	AM_DBG m_logger->debug("%s.raise_focusout_event() ST:%ld, PT:%ld, DT:%ld",
        get_sig().c_str(),
		timestamp.as_time_value_down_to(this),
		timestamp.second(),
		timestamp.as_doc_time_value());
	on_add_instance(timestamp, tn_focusout_event, timestamp.second);
}

void time_node::raise_accesskey(std::pair<qtime_type, int> accesskey) {
	qtime_type timestamp = accesskey.first;
	int ch = accesskey.second;
	timestamp.to_descendent(sync_node());
	AM_DBG m_logger->debug("%s.raise_accesskey_event(%c) ST:%ld, PT:%ld, DT:%ld",
        get_sig().c_str(),
		ch,
		timestamp.as_time_value_down_to(this),
		timestamp.second(),
		timestamp.as_doc_time_value());
	on_add_instance(timestamp, accesskey_event, timestamp.second, ch);
}

void time_node::raise_marker_event(std::pair<qtime_type, std::string> arg) {
	qtime_type timestamp = arg.first;
	std::string& name = arg.second;
	timestamp.to_descendent(sync_node());
	AM_DBG m_logger->debug("%s.raise_marker_event(%s) ST:%ld, PT:%ld, DT:%ld",
        get_sig().c_str(),
		name.c_str(),
		timestamp.as_time_value_down_to(this),
		timestamp.second(),
		timestamp.as_doc_time_value());
	on_add_instance(timestamp, tn_marker_event, timestamp.second, name);
}

void time_node::raise_state_change(std::pair<qtime_type, std::string> statearg) {
	qtime_type timestamp = statearg.first;
	std::string statevar = statearg.second;
	timestamp.to_ancestor(sync_node());
	// ??timestamp.to_node
	AM_DBG m_logger->debug("%s.raise_state_change_event(%s) ST:%ld, PT:%ld, DT:%ld",
        get_sig().c_str(),
		statevar.c_str(),
		timestamp.as_time_value_down_to(this),
		timestamp.second(),
		timestamp.as_doc_time_value());
	on_add_instance(timestamp, state_change_event, timestamp.second, statevar);
}

void time_node::raise_update_event(qtime_type timestamp) {
	m_update_event.first = true;
	m_update_event.second = timestamp;
}

///////////////////////////////
// Global operations

// Called when the beginEvent or the repeat event
// is raised by an ancestor time container.
//
// A reset operation sets this node to the same state this object had
// when it was created except that, after a reset it may contain
// in its begin or end lists some syncbased instances.
//
void time_node::reset(qtime_type timestamp, time_node *oproot) {
	// 1. Reset children with resepect to oproot.
	std::list<time_node*> children;
	get_children(children);
	std::list<time_node*>::iterator it;
	qtime_type qt = timestamp.as_qtime_down_to(this);
	for(it = children.begin(); it != children.end(); it++)
		(*it)->reset(qt, oproot);

	// 2. Reset state variables and enter the reset state
	// The following call in effect calls:
	// any_state::reset() => any_state::exit() + reset_state::enter()
	// Deactivated nodes will raise an endEvent
	// This should should not update dependents within this branch (use oproot to impl this)
	m_state->reset(timestamp, oproot);
	if(m_timer) m_timer->stop();

	// 3. Reset this instance times with resepect to oproot.
	rule_list::iterator it2;
	for(it2=m_begin_list.begin();it2!=m_begin_list.end();it2++)
		(*it2)->reset(oproot);
	for(it2=m_end_list.begin();it2!=m_end_list.end();it2++)
		(*it2)->reset(oproot);
}

// Called when a node repeats or when its deactivated (restart or nomal end)
void time_node::reset_children(qtime_type timestamp, time_node *oproot) {
	std::list<time_node*> children;
	get_children(children);
	std::list<time_node*>::iterator it;
	qtime_type qt = timestamp.as_qtime_down_to(this);
	for(it = children.begin(); it != children.end(); it++)
		(*it)->reset(qt, oproot);
}

void time_node::startup_children(qtime_type timestamp) {
	std::list<time_node*> children;
	get_children(children);
	std::list<time_node*>::iterator it;
	qtime_type qt = timestamp.as_qtime_down_to(this);
	for(it = children.begin(); it != children.end(); it++) {
		(*it)->set_state(ts_proactive, qt, this);
	}
}

void time_node::kill(qtime_type timestamp, time_node *oproot) {
	// The following will deactivate this
	// the active::exit() will propagate this opation to the children
	m_state->kill(timestamp, oproot);
}

void time_node::kill_children(qtime_type timestamp, time_node *oproot) {
	std::list<time_node*> children;
	get_children(children);
	std::list<time_node*>::iterator it;
	qtime_type qt = timestamp.as_qtime_down_to(this);
	for(it = children.begin(); it != children.end(); it++) {
		if((*it)->is_link() && oproot == (*it)->up())
			(*it)->set_state(ts_postactive, qt, oproot);
		else
			(*it)->kill(qt, oproot);
	}
}

void time_node::kill_blockers(qtime_type timestamp, time_node *oproot) {
	AM_DBG m_logger->debug("kill_blockers(%s): %d in m_begin_list", get_sig().c_str(), m_begin_list.size());
	rule_list::iterator it;
	for (it=m_begin_list.begin(); it != m_begin_list.end(); it++) {
		time_node *blocker = (*it)->get_syncbase();
		AM_DBG m_logger->debug("kill_blockers(%s): depends on %s %s",
			get_sig().c_str(), blocker->get_sig().c_str(), blocker->get_state()->name());
		if (blocker->is_alive()) {
			if ((*it)->get_syncbase_event() != tn_end) {
				// Should we fire the event?
			} else {
				blocker->set_state(ts_postactive, timestamp, blocker);
			}
		}
	}
}

// Meta model reset
// Brings this node to its initial state without propagating events
// The function resets all the state variables of this class
// To simplify verfication all variables defined by the class def are copied here.
void time_node::reset() {
    AM_DBG m_logger->debug("time_node::reset(%s)", get_sig().c_str());
	///////////////////////////
	// 1. Reset visuals

	if(m_needs_remove)
		stop_playable();

	///////////////////////////
	// 2. Reset state variables

	// Structure var
	// context_type *m_context;

	// The underlying DOM node
	// Mimimize or eliminate usage after timegraph construction
	// Structure var
	// const node *m_node;

	// Attributes parser
	// Structure var
	// time_attrs m_attrs;

	// The time type of this node
	// Structure var
	// time_container_type m_type;

	// The intrinsic time type of this node
	// Always false for time containers
	// Structure var
	// bool m_discrete;

	// The timer assigned to this node by the timegraph builder
	// lib::timer *m_timer;
	if(m_timer) m_timer->stop();

	// The lifetime state of this node.
	// Summarizes the state variables below.
	// For each state the analytic state variables below
	// take particular values.
	// time_state	*m_state;
	m_state = m_time_states[ts_reset];

	// Smil timing calculator
	// Structure var
	// time_calc *m_time_calc;

	// The current interval associated with this time node.
	// When this has not a current interval this is set to unresolved
	// interval_type m_interval;
	m_interval = interval_type::unresolved;

	// Past intervals
	// Some intervals may have not "played" but they did
	// affected the state of the model by propagating
	// time change notifications.
	// Canceled intervals do not contribute.
	// std::list<interval_type> m_history;
	m_history.clear();

	// std::list<interval_type> m_doc_history;
	m_doc_history.clear();

	// Flag set when this is active
	// e.g during the current interval
	// bool m_active;
	m_active = false;

	// Flag set when this node has finished an interval,
	// has called start against its peer playable but not stop yet.
	// e.g there maybe display effects that should be removed
	// == this has to call stop() against its peer playable.
	// bool m_needs_remove;
	m_needs_remove = false;

	// The following 2 state variables are incemented when the node is active
	// and it repeats:
	// m_rad += m_last_cdur; m_precounter++;

	// Accumulated repeat duration
	// Incremented after the completion of a simple dur
	// Last begin or repeat instance as measured by the AD timer of this node.
	// time_type m_rad;
	m_rad = 0;

	// Number of completed repeat counts
	// e.g. the current zero-based repeat index
	// long m_precounter;
	m_precounter = 0;

	// EOM flag
	// bool m_eom_flag;
	m_eom_flag = false;

	// The priority of this node
	// Applicable for excl children.
	// Structure var
	// int m_priority;

	// Paused flag
	// bool m_paused;
	m_paused = false;

	// Accumulated pause duration
	//time_type m_pad;
	m_pad = 0;

	// Register for storing pause time
	// time_type m_paused_sync_time;
	m_paused_sync_time = 0;

	// Defered flag
	// bool m_deferred;
	m_deferred = false;

	// Sync update event
	//std::pair<bool, qtime_type> m_update_event;
	m_update_event.first = false;

	// Sync rules
	// typedef std::list<sync_rule*> rule_list;

	// The begin sync rules of this node.
	// Structure var but the lists should be cleared
	// rule_list m_begin_list;
	rule_list::iterator it1;
	for(it1=m_begin_list.begin();it1!=m_begin_list.end();it1++)
		(*it1)->reset(0);

	// The end sync rules of this node.
	// Structure var but the lists should be cleared
	// rule_list m_end_list;
	for(it1=m_end_list.begin();it1!=m_end_list.end();it1++)
		(*it1)->reset(0);

	// On reset all event instances are cleared.
	// Keep a register for holding one such instance
	// The one that will start the current interval
	// time_type m_begin_event_inst;
	m_begin_event_inst = time_type::unresolved;

	// Special DOM calls sync rule of this node.
	// sync_rule *m_domcall_rule;
	if(m_domcall_rule) m_domcall_rule->reset(0);

	// The dependents of this node.
	// Use pointers to minimize the overhead when there are no dependents
	//typedef std::map<sync_event, rule_list* > dependency_map;
	// dependency_map m_dependents;

	// preventing cycles flag
	// bool m_locked;
	m_locked = false;

	// when set the associated renderer should notify for activate events
	// Structure var
	//	bool m_want_activate_events;

	// when set the associated UI should notify for accesskey events
	// Structure var
	// bool m_want_accesskey;

	// Cashed implicit duration of a continous media node (audio, video).
	// time_type m_impldur;
	m_impldur = time_type::unresolved;

	// Last calc_dur() result
	m_last_cdur = time_type::unresolved;

	// Time states
	// Structure var
	// time_state* m_time_states[ts_dead+1];

	// Structure vars
	// timing-tree bonds
	// time_node *m_parent;
	// time_node *m_next;
	// time_node *m_child;
}

///////////////////////////////
// Timing measurements

// Currently the timer:
// Starts ticking at the active duration offset when the node is activated
// Progress while this node is active and playing
// Is paused when the the node or an ancestor is paused while active
// Pauses or stops when the node is deactivated

// Returns the simple time of this node.
time_node::value_type time_node::get_simple_time() const {
	if(!m_timer) return 0;

	// The AD offset of this node
	// Any pause intervals are not included (see accumulated pause dur: pad)
	// The total time this node has been active is: ad_offset + m_pad
	time_type ad_offset = m_timer->elapsed();

	// The SD offset of this node
	time_type sd_offset = ad_offset - m_rad;
	return sd_offset();
}

// Returns the current interval end taking into account pausing
time_node::time_type time_node::get_interval_end() const {
	if(m_time_calc->uses_dur())
		return m_interval.end + m_pad;
	return m_interval.end;
}

// Returns the time elapsed since the beginning of
// the interval taking into account pausing.
// The node should be active and playing.
// When paused the value returned will not take into acount the paused period
time_node::value_type time_node::get_time() const {
	if(!m_timer) return 0;
	return m_timer->elapsed() + m_pad();
}


/////////////////////////////////////////////////////
// time_container overrides

// The implicit duration of a par is controlled by endsync.
// By default, the implicit duration of a par is defined by
// the endsync="last" semantics. The implicit duration ends
// with the last active end of the child elements.
time_node::time_type
time_container::get_implicit_dur() {
	if(!down()) return 0;
	endsync_rule esr = m_attrs.get_endsync_rule();
	time_type idur = time_type::unresolved;
	std::list<const time_node*> cl;
	get_children(cl);
	switch(esr) {
		case esr_first:
			idur =	calc_implicit_dur_for_esr_first(cl);
			break;
		case esr_last:
			idur =	calc_implicit_dur_for_esr_last(cl);
			break;
		case esr_all:
			idur =	calc_implicit_dur_for_esr_all(cl);
			break;
		case esr_id:
			idur =	calc_implicit_dur_for_esr_id(cl);
			break;
		default:
			idur =	calc_implicit_dur_for_esr_last(cl);
			break;
	}
	return idur;
}

// Returns the minimum of the children first intervals end
// If no child has a valid interval returns "unresolved"
// Played intervals count
time_node::time_type
time_container::calc_implicit_dur_for_esr_first(std::list<const time_node*>& cl) {
	std::list<const time_node*>::const_iterator it;
	time_type idur = time_type::unresolved;
	for(it=cl.begin();it!=cl.end();it++) {
		const time_node *c = *it;
		const interval_type& i = c->get_first_interval();
		if(i.is_valid())
			idur = std::min(idur, c->get_interval_end());
	}
	AM_DBG m_logger->debug("%s.calc_implicit_dur_for_esr_first(): %s",
        get_sig().c_str(),
        ::repr(idur).c_str());
	return idur;
}

// Returns the interval end of the designated child
// If the designated child has not a valid interval returns "unresolved"
time_node::time_type
time_container::calc_implicit_dur_for_esr_id(std::list<const time_node*>& cl) {
	std::list<const time_node*>::const_iterator it;
	time_type idur = time_type::unresolved;
	std::string endsync_id = m_attrs.get_endsync_id();
	for(it=cl.begin();it!=cl.end();it++) {
		const time_node *c = *it;
		if(endsync_id == c->get_time_attrs()->get_id()) {
			const interval_type& i = c->get_first_interval();
			if(i.is_valid()) idur = c->get_interval_end();
			break;
		}
	}
	AM_DBG m_logger->debug("%s.calc_implicit_dur_for_esr_id(): %s",
        get_sig().c_str(), 
        ::repr(idur).c_str());
	return idur;
}

// Returns the maximum of the children interval ends
// This maybe called before the children go live, consider this case
// The current interval may not be the first [(*it)->played() maybe true]
// This is the default for par, excl, media_cond
// If the children are alive and there isn't any valid interval, returns 0
time_node::time_type
time_container::calc_implicit_dur_for_esr_last(std::list<const time_node*>& cl) {
	std::list<const time_node*>::const_iterator it;
	time_type idur = time_type::minus_infinity;
	for(it=cl.begin();it!=cl.end();it++) {
		const time_node *c = *it;
		const interval_type& i = c->get_last_interval();
		if(!c->is_alive() || c->paused()) {
			// Building up, wait
			idur = time_type::unresolved;
			break;
		} else if(i.is_valid()) {
			idur = std::max(idur, c->get_interval_end());
		}
	}
	AM_DBG m_logger->debug("%s.calc_implicit_dur_for_esr_last(): %s",
        get_sig().c_str(), 
        ::repr(idur).c_str());
	return (idur == time_type::minus_infinity)?0:idur;
}

// Returns the maximum of the children interval ends like "last"
// The diff of "all" is that should wait for all to play at least once
time_node::time_type
time_container::calc_implicit_dur_for_esr_all(std::list<const time_node*>& cl) {
	std::list<const time_node*>::const_iterator it;
	time_type idur = time_type::minus_infinity;
	for(it=cl.begin();it!=cl.end();it++) {
		const time_node *c = *it;
		const interval_type& i = c->get_last_interval();
		if(i.is_valid())
			idur = std::max(idur, c->get_interval_end());
		else if(!c->played()) {
			idur = time_type::unresolved;
			break;
		}
	}
	AM_DBG m_logger->debug("%s.calc_implicit_dur_for_esr_all(): %s",
        get_sig().c_str(), 
        ::repr(idur).c_str());
	return (idur == time_type::minus_infinity)?time_type::unresolved:idur;
}

// Returns true when the end sync cond is applicable for this container
bool time_container::end_sync_cond_applicable() const {
	if(!m_attrs.has_dur_specifier() && m_attrs.specified_end())
		return false;
	dur_type dt = m_attrs.get_dur_type();
	if(dt != dt_unspecified && dt != dt_media)
		return false;
	return true;
}

// Returns true when the end sync cond evaluates to true
// Assumes that end sync cond is applicable
// When this returns true the container should be deactivated
bool time_container::end_sync_cond() const {
	std::list<const time_node*> cl;
	std::list<const time_node*>::const_iterator it;
	get_children(cl);

	endsync_rule esr = m_attrs.get_endsync_rule();
	if(esr == esr_first) {
		// if any child has played an interval return true
		for(it=cl.begin();it!=cl.end();it++)
			if((*it)->played()) return true;
		return false;
	} else if(esr == esr_id) {
		// if the designated child has played an interval return true
		std::string endsync_id = m_attrs.get_endsync_id();
		for(it=cl.begin();it!=cl.end();it++) {
			if(endsync_id == (*it)->get_time_attrs()->get_id()) {
				if((*it)->played()) return true;
				else break;
			}
		}
		return false;
	} else if(esr == esr_last) {
		// if any child has a valid current interval, wait it to finish (return false)
		// this is the default for par, excl, media_cond
		// the current interval may not be the first [(*it)->played() maybe true]
		// (path by Jack) If we're fast-forwarding we ignore continuous-media children
		for(it=cl.begin();it!=cl.end();it++) {
			if (m_ffwd_mode && (*it)->is_cmedia()) continue;
			const interval_type& i = (*it)->get_current_interval();
			if(i.is_valid()) return false;
		}
		return true;
	}
	else if(esr == esr_all) {
		// if any child has a valid current interval, or it has not has played yet
		// wait it to finish or start (return false)
		for(it=cl.begin();it!=cl.end();it++) {
			const interval_type& i = (*it)->get_current_interval();
			if(i.is_valid() || !(*it)->played())
				return false;
		}
		return true;
	}
	assert(false);
	return false;

}

// seq implicit_dur
// The implicit duration of a seq ends with the active end
// of the last child of the seq.
// If any child of a seq has an indefinite active duration,
// the implicit duration of the seq is also indefinite.
time_node::time_type
seq::get_implicit_dur() {
	if(!down()) return 0;
	const time_node* tn = last_child();
	const interval_type& i = tn->get_last_interval();
	time_type idur =  time_type::unresolved;
	if(i.is_valid()) idur = tn->get_interval_end();
	AM_DBG m_logger->debug("%s.get_implicit_dur(): %s",
        get_sig().c_str(), 
        ::repr(idur).c_str());
	return idur;
}

// Returns true when the end sync cond evaluates to true
// Assumes that end sync cond is applicable
// When this returns true the seq should be deactivated
bool seq::end_sync_cond() const {
	const time_node* tn = last_child();
	return tn == NULL || tn->played();
}

///////////////////////////////
// excl implementation

// Creates an excl node
excl::excl(context_type *ctx, const lib::node *n)
:	time_container(ctx, n, tc_excl),
	m_queue(new excl_queue()),
	m_num_classes(0),
	m_priority_attrs(0) {
}

// Destructs an excl node
excl::~excl() {
	delete m_queue;
	for(int i=0;i<m_num_classes;i++)
		delete m_priority_attrs[i];
	if(m_priority_attrs) delete[] m_priority_attrs;
}

// This function is called once by the timegraph
// for each excl element to build the required
// priorityClass related structures.
void excl::built_priorities() {
	AM_DBG m_logger->debug("%s.built_priorities()", get_sig().c_str());

	const node *n = m_node->down();
	if(!n || n->get_local_name() != "priorityClass") {
		m_num_classes = 1;
		m_priority_attrs = new priority_attrs_ptr[m_num_classes];
		m_priority_attrs[0] = new priority_attrs();
		return;
	}

	// create a map: node -> time_node
	std::map<const node*, time_node*> n2tn;
	std::list<time_node*> cl;
	get_children(cl);
	std::list<time_node*>::iterator it1;
	for(it1=cl.begin();it1!=cl.end();it1++)
		n2tn[(*it1)->dom_node()]= (*it1);

	// read priorityClass attributes
	std::list<const node*> prio_classes;
	m_node->get_children(prio_classes);

	// keep number of priorityClass
	m_num_classes = int(prio_classes.size());
	AM_DBG m_logger->debug("%s.built_priorities() %d classes",
        get_sig().c_str(), 
        prio_classes.size());

	// the number to be assigned to the higher priority
	int prio = m_num_classes-1;

	// Array to store prio attrs struct
	m_priority_attrs = new priority_attrs_ptr[m_num_classes];

	// Scan excl element children (some or all should be priorityClass elements)
	std::list<const node*>::const_iterator it2;
	for(it2=prio_classes.begin();it2!=prio_classes.end();it2++, prio--) {
		if((*it2)->get_local_name() != "priorityClass") {
			// not a priorityClass
			m_priority_attrs[prio] = new priority_attrs();
			continue;
		}
		// At this point the iterator (it2) is positioned on a priorityClass
		const node *priorityClassNode = (*it2);

		// Get priorityClass children (mostly time elements)
		std::list<const node*> time_children;
		std::list<const node*>::const_iterator it3;
		priorityClassNode->get_children(time_children);
		for(it3=time_children.begin();it3!=time_children.end();it3++) {
			// locate time node and set its prio
			time_node *tn = n2tn[*it3];
			if(tn) {
				tn->set_priority(prio);
				// this prio applies and to all its children

				AM_DBG m_logger->debug("%s priority: %d", get_sig().c_str(), prio);
			}
			//else not a time node
		}
		// create and store prio attrs struct
		m_priority_attrs[prio] = priority_attrs::create_instance(priorityClassNode);
	}
}

// Utility:
// Returns the current active child.
// Check that there is only one.
time_node*
excl::get_active_child() {
	std::list<time_node*> cl;
	get_children(cl);
	time_node *candidate = NULL;

	std::list<time_node*>::iterator it;
	for(it=cl.begin();it!=cl.end();it++) {
		if((*it)->is_active() && !(*it)->paused()) {
			assert(candidate == NULL);
			candidate = *it;
		}
	}
	return candidate;
}

// Utility:
// Returns the current filled child.
// Check that there is only one.
time_node*
excl::get_filled_child() {
	std::list<time_node*> cl;
	get_children(cl);
	time_node *candidate = NULL;

	std::list<time_node*>::iterator it;
	for(it=cl.begin();it!=cl.end();it++) {
		if((*it)->is_filled()) {
			assert(candidate == NULL);
			candidate = *it;
		}
	}
	return candidate;
}

// Implements the excl element behavior
void excl::interrupt(time_node *c, qtime_type timestamp) {
	time_node *interrupting_node = c;
	time_node *active_node = get_active_child();
	if(active_node == 0) {
		// Nothing really active. Kill any child that is in fill.
		active_node = get_filled_child();
		AM_DBG m_logger->debug("excl::interrupt: fillchild 0x%x", active_node);
		if (active_node) active_node->remove(timestamp);
		// Activate the new child and be done with it.
		interrupting_node->set_state(ts_active, timestamp, this);
		return;
	}
	AM_DBG m_logger->debug("excl::interrupt: active child 0x%x", active_node);

	// Retrieve priority attrs of active and interrupting nodes
	assert(interrupting_node->priority() < m_num_classes);
	priority_attrs *pai = m_priority_attrs[interrupting_node->priority()];
	assert(pai);
	assert(active_node->priority() < m_num_classes);
	priority_attrs *pa = m_priority_attrs[active_node->priority()];
	assert(pa);

	// get time attrs for debug printout
	const time_attrs* ta = active_node->get_time_attrs();
	assert(ta);
	const time_attrs* tai = interrupting_node->get_time_attrs();
	assert(tai);

	// Set interrupt attrs based on whether the interrupting node is peer, higher or lower
	interrupt_type what;
	if(interrupting_node->priority() == active_node->priority()) {
		AM_DBG m_logger->debug("%s int by peer %s",
            get_sig().c_str(),
            interrupting_node->get_sig().c_str());
		what = pa->peers;
	} else if(interrupting_node->priority() > active_node->priority()) {
		AM_DBG m_logger->debug("%s int by higher %s",
            get_sig().c_str(),
            interrupting_node->get_sig().c_str());

		what = pa->higher;
	} else {
		AM_DBG m_logger->debug("%s int by lower %s",
            get_sig().c_str(),
            interrupting_node->get_sig().c_str());
        what = pa->lower;
	}

	// handle interrupt based on excl semantics
	if(what == int_stop) {
		// stop active_node
		active_node->set_state(ts_postactive, timestamp, c);
		AM_DBG m_logger->debug("excl::interrupt: active_node get_fill() returns %d", (int)ta->get_fill());
		// Jack added the remove() call to clear the node off the screen
		if(ta->get_fill() == fill_freeze)
			active_node->remove(timestamp);

		AM_DBG m_logger->debug("%s int_stop by %s",
            get_sig().c_str(),
            interrupting_node->get_sig().c_str());
		// start interrupting_node
		//interrupting_node->set_state(ts_active, timestamp, c);
		interrupting_node->set_state(ts_active, timestamp, c);

	} else if(what == int_pause) {
		// pause active_node and and insert it on the queue
		// do not cancel interval
		// a node may end while paused
		// accepts sync_update

		active_node->pause(timestamp, pa->display);
		m_queue->push_pause(active_node);

		AM_DBG m_logger->debug("%s int_pause by %s", 
            get_sig().c_str(),
            interrupting_node->get_sig().c_str());
		// start interrupting_node
		interrupting_node->set_state(ts_active, timestamp, c);

	} else if(what == int_defer) {
		AM_DBG m_logger->debug("%s continue int_defer: %s",
            get_sig().c_str(),
            interrupting_node->get_sig().c_str());
		// do not touch active_node
		// defer the interrupting_node
		// arrange so that the deferred node ingnores sync_update (see spec)
		interrupting_node->defer_interval(timestamp);
		m_queue->push_defer(interrupting_node);

	} else {
		// what == int_never
		// do not touch active_node
		// ignore interrupting_node
		AM_DBG m_logger->debug("%s int_never ignore: %s",
            get_sig().c_str(),
            interrupting_node->get_sig().c_str());

		// assert that the interval is canceled
		// e.g. cancel notification to dependents
		interrupting_node->cancel_interval(timestamp);
		interrupting_node->set_state(ts_postactive, timestamp, c);
	}
}

// Notification for a child normal end.
void excl::on_child_normal_end(time_node *c, qtime_type timestamp) {
	if(!m_queue || m_queue->empty()) return;
	time_node *nxt = m_queue->pop();

	AM_DBG m_logger->debug("%s.dequeue %s", get_sig().c_str(), (nxt->is_active()?"active":"deferred"));

	// if its active resume else start
	if(nxt->paused()) {
		c->remove(timestamp);
		nxt->resume(timestamp);
	} else if(nxt->deferred()) {
		// When an element is deferred, the begin time is deferred as well.
		// Note that we don't need the c->remove() here, it is done later
		// in interrupt().
		nxt->set_deferred_interval(timestamp);
	} else {
		assert(false);
	}
}

// Service: removes a node from the queue of this excl
void excl::remove(time_node *tn) {
	if(!m_queue || m_queue->empty()) return;
	m_queue->remove(tn);
}

///////////////////////
// excl_queue implementation

// Pushes the provided paused node to the queue.
void excl_queue::push_pause(time_node *tn) {
	// an element may not appear in the queue more than once
	m_cont.remove(tn);

	// Paused elements are inserted before elements with the same priority
	int prio = tn->priority();
	cont::iterator it=m_cont.begin();
	while(it!=m_cont.end() &&  (*it)->priority()>prio) it++;
	assert(it==m_cont.end() || (*it)->priority()<=prio);
	m_cont.insert(it, tn);
}

// Pushes the provided defered node to the queue.
void excl_queue::push_defer(time_node *tn) {
	// an element may not appear in the queue more than once
	m_cont.remove(tn);

	// Deferred elements are inserted after elements with the same priority
	int prio = tn->priority();
	cont::iterator it = m_cont.begin();
	while(it!=m_cont.end() &&  (*it)->priority()>=prio) it++;
	assert(it==m_cont.end() || (*it)->priority()<prio);
	m_cont.insert(it, tn);
}

// Asserts queue invariants
// See spec
void excl_queue::assert_invariants() const {
	std::set<time_node*> s;
	cont::size_type count = 0;
	int prev_prio = std::numeric_limits<int>::max();
	for(cont::const_iterator it = m_cont.begin();it!=m_cont.end();it++) {
		// Assert:
		// an element may not simultaneously be active and in the queue
		assert(!(*it)->is_active());

		// Assert:
		// The queue is sorted by priority, with higher priority elements
		// before lower priority elements
		assert(prev_prio >= (*it)->priority());
		prev_prio = (*it)->priority();

		s.insert(*it);count++;
	}

	// Assert:
	// An element may not appear in the queue more than once
	assert(s.size() == count);
}

void time_node::follow_link(qtime_type timestamp) {
	const char *nohref = m_node->get_attribute("nohref");
	if(nohref && strcmp(nohref, "nohref") == 0) {
		// An anchor with nohref="nohref" does nothing.
		return;
	}
	if(m_node->get_attribute("href") == NULL) {
		return;
	}
	net::url href = m_node->get_url("href");
	const char *sourceplaystate = m_node->get_attribute("sourcePlaystate");
	const char *destinationplaystate = m_node->get_attribute("destinationPlaystate");
	const char *show = m_node->get_attribute("show");
	const char *external = m_node->get_attribute("external");
	const char *target = m_node->get_attribute("target");

	src_playstate source_state = src_replace;
	dst_playstate destination_state = dst_play;

	// Note: the order here is important
	if (sourceplaystate && strcmp(sourceplaystate, "play") == 0) {
		source_state = src_play;
	} else if (sourceplaystate && strcmp(sourceplaystate, "stop") == 0) {
		source_state = src_replace;
	} else	if (sourceplaystate && strcmp(sourceplaystate, "pause") == 0) {
		source_state = src_pause;
	}

	if (destinationplaystate && strcmp(destinationplaystate, "play") == 0) {
		destination_state = dst_play;
	} else if (destinationplaystate && strcmp(destinationplaystate, "pause") == 0) {
		destination_state = dst_pause;
	}

	if (show && strcmp(show, "new") == 0) {
		source_state = src_play;
	} else if (show && strcmp(show, "pause") == 0) {
		source_state = src_pause;
	} else if (show && strcmp(show, "replace") == 0) {
		source_state = src_replace;
	}
	// XXX Should ignore show if target is set, according to the spec. But the spec is
	// not very clear, so I have asked for clarification and don't clear show right now.

	if (external && strcmp(external, "true") == 0) {
		destination_state = dst_external;
	}

	m_context->show_link(m_node, href, source_state, destination_state, target);
}

