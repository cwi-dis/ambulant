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

#include "ambulant/smil2/time_node.h"
#include "ambulant/common/schema.h"
#include "ambulant/smil2/animate_n.h"
#include "ambulant/smil2/animate_e.h"
#include "ambulant/smil2/time_calc.h"
#include <cmath>
#include <stack>
#include <set>
#include <list>

#include "ambulant/lib/logger.h"

//#define AM_DBG if(1)

#ifndef AM_DBG
#define AM_DBG if(0)
#endif

#ifndef AMBULANT_NO_IOSTREAMS
#include <iostream>
#endif

using namespace ambulant;
using namespace smil2;

static lib::logger *tnlogger = 0;

// static
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
	m_picounter(0),
	m_active(false),
	m_needs_remove(false),
	m_last_cdur(time_type::unresolved),
	m_rad(0),
	m_rad_offset(0),
	m_precounter(0),
	m_priority(0),
	m_paused(false), 
	m_deferred(false),
	m_begin_event_inst(time_type::unresolved),
	m_pending_event(0),
	m_domcall_rule(0),
	m_locked(false),
	m_want_activate_events(false),
	m_want_accesskey(false),
	m_mediadur(time_type::unresolved),
	m_impldur(time_type::unresolved),
	m_parent(0), m_next(0), m_child(0) {
	assert(type <= tc_none);
	node_counter++;
	if(!tnlogger) tnlogger = lib::logger::get_logger();
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
		
	// This node owns the lists but not 
	// the sync_rules within the lists.
	dependency_map::iterator dit;
	for(dit=m_dependents.begin();dit!=m_dependents.end();dit++)
		delete (*dit).second;
	
	// delete this time states
	for(int i=0;i<=ts_dead;i++)
		delete m_time_states[i];
	
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
	m_rad_offset =  m_context->elapsed();
	AM_DBG tnlogger->trace("%s.startElement()", to_string().c_str());
	qtime_type timestamp(this, 0);
	m_domcall_rule->add_instance(timestamp, 0);
	schedule_state_transition(ts_proactive, timestamp, 0);
}

// DOM TimeElement::stopElement()
// Currently supported only for the root.
void time_node::stop() {
	m_context->cancel_all_events();
	qtime_type timestamp(sync_node(), get_sync_simple_time());
	reset(timestamp, this);
}

// DOM TimeElement::pauseElement()
void time_node::pause() {
}

bool time_node::is_animation() const {
	return common::schema::get_instance()->is_animation(m_node->get_qname());
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

// Adds a sync arc from this node to a target
void time_node::add_dependent(sync_rule *sr, sync_event ev) {
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

// Returns the implicit duration of this node. 
// This is an implementation for a leaf-node.
// e.g. queries playable for the implicit dur.
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
	if(is_discrete()) return time_type(0);
	
	// Was the implicit duration calculated before?
	// If yes, avoid the overhead of querring
	if(m_impldur != time_type::unresolved)
		return m_impldur;	
	
	// Can the associated playable provide us the implicit duration? 
	// If yes, store it a "m_mediadur" in internal timing units (ms).
	//
	// Please note that the current implementation,
	// when the duration is not defined explicitly,
	// and in order to make slideshows more smooth,
	// it postpones setting the implicit duration
	// until an end of media (EOM) event.
	// This makes the model more complex but i think
	// the results deserve this extra complexity.
	// Remains to be proved ...
	//
	if(m_mediadur == time_type::unresolved) {
		std::pair<bool, double> dur_pair = m_context->get_dur(m_node);
		if(dur_pair.first && dur_pair.second>0) {
			m_mediadur = secs_to_time_type(dur_pair.second)();
			AM_DBG tnlogger->trace("%s[%s].media_dur(): %ld", m_attrs.get_tag().c_str(), 
				m_attrs.get_id().c_str(), m_mediadur());
		}	
	}
	
	// No, the playable cannot provide its implicit duration
	AM_DBG tnlogger->trace("%s[%s].get_implicit_dur(): unresolved", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str());
		
	return time_type::unresolved;
}

// This function calculates the simple duration of this node.
// See spec: "Defining the simple duration" 
// The last calculated simple duration is stored in the variable m_last_cdur.
// This function will call the function "get_implicit_dur()" if and only if 
// the implicit duration of a node is required by the timing model.
time_node::time_type 
time_node::calc_dur() {
	return (m_last_cdur = m_time_calc->calc_dur(), m_last_cdur);
}

// Returns true for continous media leaf nodes for which
// the implicit duration is required by the model for
// timing calculations.
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
time_node::time_type 
time_node::calc_current_interval_end()  {
	time_mset end_list;
	get_instance_times(m_end_list, end_list);
	return m_time_calc->calc_interval_end(m_interval, end_list);
}

// Calculates the first valid interval for this node. 
time_node::interval_type 
time_node::calc_first_interval() {

	// Get the begin instance list
	time_mset begin_list;
	get_instance_times(m_begin_list, begin_list);

	// Get the end instance list
	time_mset end_list;
	get_instance_times(m_end_list, end_list);
	
	// Parent simple duration
	time_type parent_simple_dur = up()?up()->get_last_dur():time_type::indefinite;

	return m_time_calc->calc_first_interval(begin_list, end_list, parent_simple_dur);
}

// Calculates the next acceptable interval for this node.
// The variable this->m_interval holds the just ended interval.
time_node::interval_type 
time_node::calc_next_interval() {
	
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
	
	// The next valid interval must begin after previous end
	time_type begin_after = m_interval.end;

	// Parent simple dur
	time_type parent_simple_dur = up()?up()->get_last_dur():time_type::indefinite;
	
	return m_time_calc->calc_next_interval(begin_list, end_list, parent_simple_dur, begin_after, 
		m_interval.is_zero_dur());
}

// Sets the state of this node.
// timestamp: "scheduled now" in parent simple time
void time_node::set_state(time_state_type state, qtime_type timestamp, time_node *oproot) {
	if(m_state->ident() == state) return;
	m_state->exit(timestamp, oproot);
	m_state = m_time_states[state];
	m_state->enter(timestamp);
}

// Cancels the current interval.
void time_node::cancel_interval(qtime_type timestamp) {
	AM_DBG tnlogger->trace("%s[%s].cancel_interval(): %s", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), ::repr(m_interval).c_str());	
	assert(m_interval.is_valid());
	
	// The interval should be updated before sync_update 
	// to make available the new info to induced calcs
	interval_type i = m_interval;
	m_interval = interval_type::unresolved;	
	
	on_cancel_instance(timestamp, tn_begin, i.begin);
	on_cancel_instance(timestamp, tn_end, i.end);

	cancel_schedule();
}

// Updates the current interval with the one provided.
void time_node::update_interval(qtime_type timestamp, const interval_type& new_interval) {
	AM_DBG tnlogger->trace("%s[%s].update_interval(): %s -> %s", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), ::repr(m_interval).c_str(), ::repr(new_interval).c_str());	
	assert(m_interval.is_valid());
	assert(timestamp.first == sync_node());
	
	// The interval should be updated before sync_update 
	// to make available the new info to induced calcs
	interval_type old = m_interval;
	m_interval = new_interval;	
	
	if(m_interval.begin != old.begin) {
		time_type dt = m_interval.begin - timestamp.second;
		qtime_type qt(sync_node(), m_interval.begin);
		on_update_instance(timestamp, tn_begin, m_interval.begin, old.begin);
		schedule_state_transition(ts_active, qt, dt);
	} 
	if(m_interval.end != old.end) {
		time_type dt = m_interval.end - timestamp.second;
		if(dt.is_definite()) {
			// Sync node is probably interested for this event.
			if(up()) up()->schedule_sync_update(timestamp, 0);
		}
		on_update_instance(timestamp, tn_end, m_interval.end, old.end);
	}
}

// Updates the current interval end with the value provided.
void time_node::update_interval_end(qtime_type timestamp, time_type new_end) {
	AM_DBG tnlogger->trace("%s[%s].update_interval_end(): %s -> %s at PT:%ld, DT:%ld", 
		m_attrs.get_tag().c_str(), m_attrs.get_id().c_str(), 
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
	if(up()) up()->schedule_sync_update(timestamp, 0);
}

// Cancels the schedule of this node.
void time_node::cancel_schedule() {
	if(m_pending_event) {
		m_context->cancel_event(m_pending_event);
		m_pending_event = 0;
	}
}

// Sets a new interval as current, updates dependents and schedules activation.
// After this call the state of the node 
// a) Remains the same (proactive or postactive) if the interval is after timestamp.
//    In this case the interval is scheduled.
// b) Transitions to postactive if the interval is before timestamp
// c) Transitions to active if the interval contains timestamp
// See active_state::enter() for the activities executed
// when the node is activated.
// param timestamp: "now" in parent simple time
void time_node::schedule_interval(qtime_type timestamp, const interval_type& i) {
	AM_DBG tnlogger->trace("%s[%s].schedule_interval(): %s (DT=%ld)", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), ::repr(i).c_str(), timestamp.as_doc_time_value());
	
	// verify the assumptions made in the following code
	assert(timestamp.first == sync_node());
	assert(m_state->ident() == ts_proactive || m_state->ident() == ts_postactive);
	assert(is_root() || up()->m_state->ident() == ts_active);
	
	// Set the interval as current.
	m_interval = i;
	
	// Update dependents, event if this interval will never play
	on_new_instance(timestamp, tn_begin, m_interval.begin);
	on_new_instance(timestamp, tn_end, m_interval.end);
	

	// Update parent to recalc end sync status
	if(sync_node()->is_par() || sync_node()->is_excl())
		sync_node()->schedule_sync_update(timestamp, 0);
	
	// Is this a cut-off interval?
	// this can happen when proactive
	if(m_interval.before(timestamp.second)) {
		
		assert(m_state->ident() == ts_proactive);
		
		// notifications have been sent
		// increment past intervals counter
		// transition directly to postactive
		// (see active_state::exit conditions)
		m_picounter++;
		
		// Jump to post active
		set_state(ts_postactive, timestamp, this);
		return; 
	}
		
	if(m_interval.after(timestamp.second)) {
		// Schedule activation: 
		// activate at m_interval.begin - timestamp
		// remain in the proactive state waiting interval to begin
		time_type dt = m_interval.begin - timestamp.second;
		qtime_type qt(sync_node(), m_interval.begin);
		schedule_state_transition(ts_active, qt, dt);
		return; 
	}
	
	// Else, the interval should be activated now
	assert(m_interval.contains(timestamp.second));
	if(deferred()) defer_interval(timestamp);
	else set_state_ex(ts_active, timestamp);
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
	// t_p = t_c + rad_c + begin_c  => rad_c + t_c = t_p - begin_c
	// => t_c = rem(t_p - begin_c, dur) and rad_c = mod(t_p - begin_c, dur)*dur
	
	// The offset we are now within the current interval
	time_type ad_offset = timestamp.second - m_interval.begin;
	
	// The simple duration of this node
	time_type cdur = calc_dur();
	
	// Calculate the offset within the simple duration
	// that this node should start playing.
	time_type sd_offset;
	if(ad_offset == 0) {
		sd_offset = 0;
	} else if(!cdur.is_definite()) {
		sd_offset = ad_offset;
	} else if(cdur == 0) {
		sd_offset = 0;
	} else {
		sd_offset = ad_offset.rem(cdur);
		
		// In this case we need to update the values of the repeat registers.
		// Previous values: m_rad(0), m_rad_offset(0), m_precounter(0)
		
		// The current repeat index
		m_precounter = ad_offset.mod(cdur);
		
		// The accumulated repeat interval.
		m_rad = m_precounter*cdur();
		
		// The accumulated repeat interval as a parent simple time instance
		m_rad_offset = qtime_type::to_sync_time(this, m_rad)();
	}
		
	// Store the the activation time
	// Required by the set-of-implicit-dur-on-eom mechanism
	m_activation_time = timestamp.second; 
	
	m_timer->set_time(sd_offset());
	
	AM_DBG tnlogger->trace("%s[%s].start(%ld) ST:%ld, PT:%ld, DT:%ld", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(),  sd_offset(), sd_offset(),
		timestamp.second(),
		timestamp.as_doc_time_value());
	
	// If this is a leaf node start playable at 'sd_offset' within media
	if(!is_time_container()) {
		if(!is_discrete() && needs_implicit_dur() && m_mediadur != time_type::unresolved) {
			// we need this due to the set-of-implicit-dur-on-eom mechanism
			m_media_offset = sd_offset.rem(m_mediadur);
		} else {
			m_media_offset = sd_offset;
		}
		m_timer->set_time(m_media_offset());
		if(paused()) {
			m_pause_time = m_media_offset;
			schedule_next_timer_event(timestamp);
			return;
		}
		if(is_animation()) {
			animation_engine *ae = m_context->get_animation_engine();
			animate_node *an = (animate_node*)this;
			an->prepare_interval();
			ae->started(an);
		} else {
			common::playable *np = m_context->create_playable(m_node);
			if(np) {
				np->wantclicks(m_want_activate_events);
				const lib::node *trans_in = m_attrs.get_trans_in();
				if(trans_in) {
					m_context->start_playable(m_node, time_type_to_secs(m_media_offset()), trans_in);
				} else
					np->start(time_type_to_secs(m_media_offset()));
				AM_DBG tnlogger->trace("%s[%s].start playable(%ld) ST:%ld, PT:%ld, DT:%ld", m_attrs.get_tag().c_str(), 
					m_attrs.get_id().c_str(),  sd_offset(), sd_offset(),
					timestamp.second(),
					timestamp.as_doc_time_value());
				
			}
		}
	} 
	if(paused()) m_pause_time = sd_offset;
	else if(is_animation() || !is_cmedia()) {
		m_timer->set_speed(m_attrs.get_speed());
		m_timer->resume();
	}
	// else wait bom from playable
	
	// Schedule a check for the next S-transition e.g. repeat or end.
	// Requires: m_interval, m_last_cdur, m_rad_offset  
	schedule_next_timer_event(timestamp);
}

// Timer event callback for this node.
// This callback is called while this node is active.
// The code is responsible to check the conditions
// and apply any S-transitions for this node.
void time_node::timer_event_callback(const timer_event *e) {
	// ignore canceled events
	if(m_pending_event != (lib::event*)e) return;
	m_pending_event = 0;
	
	qtime_type timestamp =  e->m_timestamp;
	
	// the following should be true
	assert(timestamp.first == sync_node());
	
	// Check for the EOI event
	if(is_active() && timestamp.second >= m_interval.end) {
		qtime_type qt(sync_node(), m_interval.end);
		set_state_ex(ts_postactive, qt);
		return;
	} 

	// The AD offset of this node
	time_type ad_offset = timestamp.second - m_interval.begin;
	
	// The SD offset of this node
	time_type sd_offset;
	if(ad_offset == 0) {
		sd_offset = 0;
	} else if(!m_last_cdur.is_definite()) {
		sd_offset = ad_offset;
	} else if(m_last_cdur == 0) {
		sd_offset = 0;
	} else {
		sd_offset = timestamp.second - m_rad_offset;
	}
	
	
	// Check for the EOSD event
	if(m_last_cdur.is_definite() && m_last_cdur != 0 && sd_offset >= m_last_cdur) {
		// may call repeat
		on_eosd(timestamp);
	}
	
	// Finally:
	if(is_active()) {
		schedule_next_timer_event(timestamp);
	}
}

// Schedules a state transition to happen at dt from now.
void time_node::schedule_state_transition(time_state_type tst, qtime_type timestamp, time_type dt) {
	if(m_pending_event) m_context->cancel_event(m_pending_event);
	assert(dt.is_definite());
	m_pending_event = new transition_event(this, tst, timestamp);
	m_context->schedule_event(m_pending_event, dt());
}

// Schedules the next timer event for this node.
// Requires: m_interval, m_last_cdur, m_rad_offset  
void time_node::schedule_next_timer_event(qtime_type timestamp) {
	if(m_pending_event) m_context->cancel_event(m_pending_event);
	
	// The AD offset of this node
	time_type ad_offset = timestamp.second - m_interval.begin;
	
	// The SD offset of this node
	time_type sd_offset;
	if(ad_offset == 0) {
		sd_offset = 0;
	} else if(!m_last_cdur.is_definite()) {
		sd_offset = ad_offset;
	} else if(m_last_cdur == 0) {
		sd_offset = 0;
	} else {
		sd_offset = timestamp.second - m_rad_offset;
	}
	
	// Time remaining for the EOI event
	time_type ad_rest = time_type::unresolved;
	if(m_interval.end.is_definite() && m_interval.end > timestamp.second) {
		ad_rest = m_interval.end - timestamp.second;
	} 
	
	// Time remaining for the EOSD event
	time_type sd_rest = time_type::unresolved;
	if(m_last_cdur.is_definite() && m_last_cdur != 0 && m_last_cdur > sd_offset) {
		sd_rest = m_last_cdur - sd_offset;
	}

	// The next notification should not come later than min(ad_rest, sd_rest)
	time_type next = std::min(time_type(sampling_delta), std::min(ad_rest, sd_rest));
	assert (next() <= sampling_delta);
	if(next() <=0) next = 0;
	m_pending_event = new timer_event(this, timestamp + next);
	m_context->schedule_event(m_pending_event, next());
}

// Schedules a sync update notification to happen at dt from now.
void time_node::schedule_sync_update(qtime_type timestamp, time_type dt) {
	typedef scalar_arg_callback_event<time_node, qtime_type> sync_update_cb;
	sync_update_cb *cb = new sync_update_cb(this, &time_node::sync_update, timestamp);
	m_context->schedule_event(cb, dt());
}


// Scheduled transition callback
void time_node::state_transition_callback(const transition_event *e) {
	// ignore canceled events
	if(m_pending_event != (lib::event*)e) return;
	m_pending_event = 0;
	set_state_ex(e->m_state, e->m_timestamp);
}

// Calls set_state() after checking for excl
void time_node::set_state_ex(time_state_type tst, qtime_type timestamp) {
	// this should be true
	assert(timestamp.first == sync_node());
	
	//if(!is_root() && !is_alive())
	//	return; // not an S transition
	
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


// Called on the end of simple duration event
void time_node::on_eosd(qtime_type timestamp) {
	AM_DBG tnlogger->trace("*** %s[%s].on_eosd() ST:%ld, PT:%ld, DT:%ld (sdur=%ld)", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), 
		timestamp.as_time_value_down_to(this),
		timestamp.second(), 
		timestamp.as_doc_time_value(),
		m_last_cdur()
		);
	
	// update repeat registers
	m_rad += m_last_cdur();
	m_precounter++;	
	m_rad_offset = timestamp.second();
	
	// Should this node repeat?
	
	if(m_attrs.specified_rdur()) {
		time_type rdur = m_attrs.get_rdur();
		if(rdur == time_type::indefinite || rdur > m_rad) {
			repeat(timestamp);
			return;
		}
	}
	
	if(m_attrs.specified_rcount()) {
		double rcount = m_attrs.get_rcount();
		if(m_attrs.is_rcount_indefinite() || rcount > double(m_precounter)) {
			repeat(timestamp);
			return;
		}
	}
}

// The following function is called when the node should repeat.
// It is responsible to execute the repeat actions for this node. 
void time_node::repeat(qtime_type timestamp) {
	AM_DBG tnlogger->trace("*** %s[%s].repeat() ST:%ld, PT:%ld, DT:%ld", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), 
		timestamp.as_time_value_down_to(this),
		timestamp.second(), 
		timestamp.as_doc_time_value());
	
	// raise_repeat_event async
	raise_repeat_event_async(timestamp);
			
	if(down()) {
		reset_children(timestamp, this);
		startup_children(timestamp);
	} 
	
	if(!is_time_container()) {
		m_context->start_playable(m_node, 0);
	}
}


// Pauses this node.
// Excl element handling.
void time_node::pause(qtime_type timestamp, pause_display d) {
	std::list<time_node*> children;
	get_children(children);
	time_type self_simple_time = timestamp.as_time_down_to(this);
	qtime_type qt(this, self_simple_time);
	std::list<time_node*>::iterator it;
	for(it = children.begin(); it != children.end(); it++)
		(*it)->pause(qt, d);
	if(!is_time_container() && is_active()) {
		m_pause_time = self_simple_time;
		m_context->stop_playable(m_node);
	}
	set_paused(true);
}

// Resumes this node.
// Excl element handling.
void time_node::resume(qtime_type timestamp) {
	set_paused(false);
	if(!is_time_container() && is_active()) {
		m_context->start_playable(m_node, time_type_to_secs(m_pause_time()));
	}
	std::list<time_node*> children;
	get_children(children);
	time_type self_simple_time = timestamp.as_time_down_to(this);
	qtime_type qt(this, self_simple_time);
	std::list<time_node*>::iterator it;
	for(it = children.begin(); it != children.end(); it++)
		(*it)->resume(qt);
	sync_update(timestamp);
}

// Defers the interval of this node.
// Excl element handling.
void time_node::defer_interval(qtime_type timestamp) {
	std::list<time_node*> children;
	get_children(children);
	time_type self_simple_time = timestamp.as_time_down_to(this);
	qtime_type qt(this, self_simple_time);
	std::list<time_node*>::iterator it;
	for(it = children.begin(); it != children.end(); it++)
		(*it)->defer_interval(qt);
		
	// Mark this node as deferred.
	set_deferred(true);
	
	// Cancel notifications
	interval_type i = m_interval;
	if(!i.is_valid()) return;
	
	m_interval = interval_type::unresolved;	
	on_cancel_instance(timestamp, tn_begin, i.begin);
	on_cancel_instance(timestamp, tn_end, i.end);
	
	// cancel schedule
	cancel_schedule();
	
	// Remember interval
	m_interval = i;
}

// Excl element handling.
// When an element is deferred, the begin time is deferred as well
void time_node::schedule_deferred_interval(qtime_type timestamp) {
	if(!deferred()) return;
	
	// Remove deferred marker
	set_deferred(false);
	
	if(!m_interval.is_valid()) return;
	
	// Translate the defered interval to timestamp
	interval_type i = m_interval;
	i.translate(timestamp.second-m_interval.begin);
	
	// Schedule interval
	schedule_interval(timestamp, i);
	
	std::list<time_node*> children;
	get_children(children);
	std::list<time_node*>::iterator it;
	time_type self_simple_time = timestamp.as_time_down_to(this);
	qtime_type qt(this, self_simple_time);
	for(it = children.begin(); it != children.end(); it++)
		(*it)->schedule_deferred_interval(qt);
}

// This function is called always when a node exits the active state,
// and is responsible to apply the appropriate fill behavior for this node.
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
	fill_behavior pfb = sync_node()->get_time_attrs()->get_fill();
	
	bool keep = (fb != fill_remove);
	
	if(keep) {
		// this node should be freezed
		AM_DBG tnlogger->trace("*** %s[%s].pause() ST:%ld, PT:%ld, DT:%ld", m_attrs.get_tag().c_str(), 
			m_attrs.get_id().c_str(),  
			timestamp.as_time_value_down_to(this), timestamp.second(), 
			timestamp.as_doc_time_value());
		if(down()) {
			std::list<time_node*> cl;
			get_children(cl);
			std::list<time_node*>::iterator it;
			time_type self_simple_time = timestamp.as_time_down_to(this);
			qtime_type qt(this, self_simple_time);
			for(it = cl.begin(); it != cl.end(); it++)
				(*it)->fill(qt);
		} 
		m_timer->pause();
		if(!is_time_container() && !is_animation()) {
			m_context->pause_playable(m_node);
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
	AM_DBG tnlogger->trace("*** %s[%s].stop() ST:%ld, PT:%ld, DT:%ld", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(),  
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
	if(is_animation()) {
		animation_engine *ae = m_context->get_animation_engine();
		ae->stopped((animate_node*)this);
	} else if(!is_time_container()) {
		m_context->stop_playable(m_node);
	}
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
// beginEvent, repeat event, endEvent for leaf nodes

// Called when this node transitions to active.
// (active_state::enter() activity)
// Containers have to reset children, bring them to life 
// and notify dependents.
// Leaf nodes have to notify dependents and start their peer playable.
void time_node::raise_begin_event(qtime_type timestamp) {
	AM_DBG tnlogger->trace("%s[%s].raise_begin_event() ST:%ld, PT:%ld, DT:%ld", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), 
		timestamp.as_time_value_down_to(this),
		timestamp.second(), 
		timestamp.as_doc_time_value());
	assert(timestamp.first == sync_node());
	on_add_instance(timestamp, tn_begin_event, timestamp.second);
	if(is_root()) m_context->started_playback();
	
	// Simulate a timegraph-sampling implementation
	// For a time container we know that more info will be available
	// immediately after its starts.  
	if(down() && m_interval.end == time_type::indefinite)
		schedule_sync_update(timestamp, 0);
}

// Called when this node repeats.
// Containers have to reset children, bring them to life 
// and notify dependents.
// Leaf nodes have to notify dependents and start their peer playable at 0.
// timestamp is parent's simple time when this event occurs.
void time_node::raise_repeat_event(qtime_type timestamp) {
	AM_DBG tnlogger->trace("%s[%s].raise_repeat_event(%d) ST:%ld, PT:%ld, DT:%ld", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), m_precounter,
		timestamp.as_time_value_down_to(this),
		timestamp.second(), 
		timestamp.as_doc_time_value());
	assert(timestamp.first == sync_node());
	on_add_instance(timestamp, tn_repeat_event, timestamp.second, m_precounter);
}

// Called when this node exits active.
// On end, containers have to kill their children 
// and notify dependents.
// Leaf nodes have to notify dependents and pause playable
void time_node::raise_end_event(qtime_type timestamp, time_node *oproot) {
	AM_DBG tnlogger->trace("%s[%s].raise_end_event() ST:%ld, PT:%ld, DT:%ld", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), 
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
	
	// Check parent end_sync conditions
	// call schedule_sync_update(timestamp) if needed
	if(up() && up()->needs_end_sync_update(this, timestamp))
		up()->schedule_sync_update(timestamp, 0);
		
	if(is_root()) m_context->done_playback();
}

void time_node::raise_activate_event(qtime_type timestamp) {
	timestamp.to_descendent(sync_node());
	AM_DBG tnlogger->trace("%s[%s].raise_activate_event() ST:%ld, PT:%ld, DT:%ld", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), 
		timestamp.as_time_value_down_to(this),
		timestamp.second(), 
		timestamp.as_doc_time_value());
	on_add_instance(timestamp, tn_activate_event, timestamp.second);
	if(is_area()) {
		const char *href = m_node->get_attribute("href");
		if(href && strcmp(href, "nohref") != 0)
			m_context->show_link(m_node, href);
	}
}

void time_node::raise_accesskey(std::pair<qtime_type, int> accesskey) {
	qtime_type timestamp = accesskey.first;
	int ch = accesskey.second;
	timestamp.to_descendent(sync_node());
	AM_DBG tnlogger->trace("%s[%s].raise_activate_event() ST:%ld, PT:%ld, DT:%ld", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), 
		timestamp.as_time_value_down_to(this),
		timestamp.second(), 
		timestamp.as_doc_time_value());
	on_add_instance(timestamp, accesskey_event, timestamp.second, ch);
}

///////////////////////////////
// Async versions of raising begin, end and repeat events
// The purpose of these is to break long chains to smaller ones
// Makes UI handling more smooth. 

void time_node::raise_begin_event_async(qtime_type timestamp) {
	typedef scalar_arg_callback_event<time_node, qtime_type> begin_event_cb;
	begin_event_cb *cb = new begin_event_cb(this, &time_node::raise_begin_event, timestamp);
	m_context->schedule_event(cb, 0);
}

void time_node::raise_repeat_event_async(qtime_type timestamp) {
	typedef scalar_arg_callback_event<time_node, qtime_type> repeat_event_cb;
	repeat_event_cb *cb = new repeat_event_cb(this, &time_node::raise_repeat_event, timestamp);
	m_context->schedule_event(cb, 0);
}

void time_node::raise_end_event_async(qtime_type timestamp, time_node *oproot) {
	typedef scalar_arg2_callback_event<time_node, qtime_type, time_node*> end_event_cb;
	end_event_cb *cb = new end_event_cb(this, &time_node::raise_end_event, timestamp, oproot);
	m_context->schedule_event(cb, 0);
}

void time_node::activate_async(qtime_type timestamp) {
	typedef scalar_arg_callback_event<time_node, qtime_type> cb_t;
	cb_t *cb = new cb_t(this, &time_node::activate, timestamp);
	m_context->schedule_event(cb, 0);
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
	// nodes that exit active will raise an endEvent 
	// This should should not update dependents within this branch
	m_state->reset(timestamp, oproot);	
	
	// 3. Reset this instance times with resepect to oproot.
	rule_list::iterator it2;
	for(it2=m_begin_list.begin();it2!=m_begin_list.end();it2++)
		(*it2)->reset(oproot);
	for(it2=m_end_list.begin();it2!=m_end_list.end();it2++)
		(*it2)->reset(oproot);
}

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
	for(it = children.begin(); it != children.end(); it++)
		(*it)->set_state(ts_proactive, qt, this);
}

void time_node::kill(qtime_type timestamp, time_node *oproot) {
	std::list<time_node*> children;
	get_children(children);
	std::list<time_node*>::iterator it;
	qtime_type qt = timestamp.as_qtime_down_to(this);
	for(it = children.begin(); it != children.end(); it++)
		(*it)->kill(qt, oproot);
	m_state->kill(timestamp, oproot);	
}

void time_node::kill_children(qtime_type timestamp, time_node *oproot) {
	std::list<time_node*> children;
	get_children(children);
	std::list<time_node*>::iterator it;
	qtime_type qt = timestamp.as_qtime_down_to(this);
	for(it = children.begin(); it != children.end(); it++) {
		if((*it)->is_area() && oproot == (*it)->up())
			(*it)->set_state(ts_postactive, qt, oproot);
		else
			(*it)->kill(qt, oproot);
	}
}

// The timestamp clock can be any
// Convert it if possible to this sync_node clock time
void time_node::sync_update(qtime_type timestamp) {
	time_type pt = timestamp.as_node_time(sync_node());
	if(pt.is_resolved()) {
		timestamp = qtime_type(sync_node(), pt);
		m_state->sync_update(timestamp);
	}
}

// Begin of media notification
// Currently this notification is not used.
// Could be used to define to slip sync offset.
void time_node::on_bom(qtime_type timestamp) {
	if(!is_discrete()) {
		qtime_type pt = timestamp.as_qtime_down_to(sync_node());
		qtime_type st = pt.as_qtime_down_to(this);
		AM_DBG tnlogger->trace("%s[%s].on_bom() ST:%ld, PT:%ld, DT:%ld", 
			m_attrs.get_tag().c_str(), 
			m_attrs.get_id().c_str(), 
			st.second(),
			pt.second(),
			timestamp.second()); 
	}
	m_timer->resume();
}

// Notification from the playable that has paused for fetching bits
void time_node::on_pom(qtime_type timestamp) {
	m_timer->pause();
}

// Notification from the playable that has resumed playback
void time_node::on_rom(qtime_type timestamp) {
	m_timer->resume();
}

// End of nedia notification
// This notification is taken into account when this node is still active
// and the implicit duration is involved in timing calculations.
void time_node::on_eom(qtime_type timestamp) {
	if(is_discrete() || !is_active() || !needs_implicit_dur())
		return;
		
	if(m_impldur != time_type::unresolved)
		return;
	
	// 	
	qtime_type pt = timestamp.as_qtime_down_to(sync_node());
	qtime_type st = pt.as_qtime_down_to(this);
	AM_DBG tnlogger->trace("%s[%s].on_eom() ST:%ld, PT:%ld, DT:%ld", 
		m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), 
		st.second(),
		pt.second(),
		timestamp.second()); 
	
	// The new knowledge we have just acquired.
	if(m_mediadur == time_type::unresolved)
		m_mediadur =  pt.second - m_activation_time;
	m_impldur = m_mediadur;
		
	// Knowing the above calc current interval
	time_type end = calc_current_interval_end();
			
	// Do deferred calculation of activation registers
	time_type ad_offset = m_activation_time - m_interval.begin;
	time_type cdur = m_last_cdur;
	if(ad_offset != 0 && cdur.is_definite() && cdur != 0) {
		// The current repeat index
		m_precounter = ad_offset.mod(cdur);
		
		// The accumulated repeat interval.
		m_rad = m_precounter*cdur();
		
		// The accumulated repeat interval as a parent simple time instance
		m_rad_offset = qtime_type::to_sync_time(this, m_rad)();
	}
	
	// Update interval end	
	if(end != m_interval.end) {		
		// The model instance of EOM
		time_type model_time = m_activation_time + (m_mediadur - m_media_offset);
						
		// Update interval refering the model instance.
		update_interval_end(qtime_type(sync_node(), model_time), end);
	}
	
}

///////////////////////////////
// Timing operations

// Returns the simple time of this sync node (parent).
time_node::value_type time_node::get_sync_simple_time() const {
	return is_root()?m_context->elapsed():sync_node()->get_simple_time();
}

// Returns the simple time of this node.
time_node::value_type time_node::get_simple_time() const {
	return get_sync_simple_time() - m_rad_offset;
}

/////////////////////////////////////////////////////
// time_container overrides

///////////////////////////////
// time_container implicit_dur

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
			idur = 	calc_implicit_dur_for_esr_first(cl);
			break;
		case esr_last:
			idur = 	calc_implicit_dur_for_esr_last(cl);
			break;
		case esr_all:
			idur = 	calc_implicit_dur_for_esr_all(cl);
			break;
		case esr_id:	
			idur = 	calc_implicit_dur_for_esr_id(cl);
			break;	
		default:
			idur = 	calc_implicit_dur_for_esr_last(cl);
			break;
	}
	return idur;
}

time_node::time_type 
time_container::calc_implicit_dur_for_esr_first(std::list<const time_node*>& cl) {
	std::list<const time_node*>::const_iterator it;
	time_type idur = time_type::unresolved;
	for(it=cl.begin();it!=cl.end();it++) {
		const time_node *c = *it;
		const interval_type& i = c->get_interval();
		if(i.is_valid())
			idur = std::min(idur, i.end);
	}
	AM_DBG tnlogger->trace("%s[%s].calc_implicit_dur_for_esr_first(): %s", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), ::repr(idur).c_str());		
	return idur;	
}

time_node::time_type 
time_container::calc_implicit_dur_for_esr_last(std::list<const time_node*>& cl) {
	std::list<const time_node*>::const_iterator it;
	time_type idur = time_type::minus_infinity;
	for(it=cl.begin();it!=cl.end();it++) {
		const time_node *c = *it;
		const interval_type& i = c->get_interval();
		if(!c->is_alive()) {
			idur = time_type::unresolved;
			break; 
		} else if(i.is_valid()) {
			idur = std::max(idur, i.end);
		}
	}
	AM_DBG tnlogger->trace("%s[%s].calc_implicit_dur_for_esr_last(): %s", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), ::repr(idur).c_str());		
	return (idur == time_type::minus_infinity)?0:idur;	
}

time_node::time_type 
time_container::calc_implicit_dur_for_esr_all(std::list<const time_node*>& cl) {
	std::list<const time_node*>::const_iterator it;
	time_type idur = time_type::minus_infinity;
	for(it=cl.begin();it!=cl.end();it++) {
		const time_node *c = *it;
		const interval_type& i = c->get_interval();
		if(i.is_valid())
			idur = std::max(idur, i.end);
		else {
			idur = time_type::unresolved;
			break;
		}
	}
	AM_DBG tnlogger->trace("%s[%s].calc_implicit_dur_for_esr_all(): %s", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), ::repr(idur).c_str());		
	return (idur == time_type::minus_infinity)?time_type::unresolved:idur;	
}

time_node::time_type 
time_container::calc_implicit_dur_for_esr_id(std::list<const time_node*>& cl) {
	std::list<const time_node*>::const_iterator it;
	time_type idur = time_type::unresolved;
	for(it=cl.begin();it!=cl.end();it++) {
		const time_node *c = *it;
		if(m_attrs.get_endsync_id() == c->get_time_attrs()->get_id()) {
			const interval_type& i = c->get_interval();
			if(i.is_valid()) idur = i.end;
			break;
		}
	}
	AM_DBG tnlogger->trace("%s[%s].calc_implicit_dur_for_esr_id(): %s", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), ::repr(idur).c_str());		
	return idur;	
}

// c: the child just de-activated
bool time_container::needs_end_sync_update(const time_node *c, qtime_type timestamp) const {
	
	// This does not need a sync_update if the implicit duration
	// will not be involved in interval calculations
	if(!m_attrs.has_dur_specifier() && m_attrs.specified_end())
		return false;
	dur_type dt = m_attrs.get_dur_type();
	if(dt != dt_unspecified && dt != dt_media)
		return false;
	
	// OK, implicit duration is involved in interval calcs
	
	endsync_rule esr = m_attrs.get_endsync_rule();
	if(esr == esr_first) return true;
	else if(esr == esr_id) {
		// do re-calc if the child just de-activated is the one specified by endsync
		return m_attrs.get_endsync_id() == c->get_time_attrs()->get_id();
	}
	else if(esr == esr_last) {
		std::list<const time_node*> cl;
		std::list<const time_node*>::const_iterator it;
		get_children(cl);
		for(it=cl.begin();it!=cl.end();it++) {
			const time_node *ac = *it;
			const interval_type& i = ac->get_interval();
			if(ac->is_active() || (i.is_valid() && !ac->has_started())) {
				// No need for re-calc
				// Wait child to go postactive or play its interval
				return false;
			}
		}
	}
	else if(esr == esr_all) {
		std::list<const time_node*> cl;
		std::list<const time_node*>::const_iterator it;
		get_children(cl);
		for(it=cl.begin();it!=cl.end();it++) {
			const time_node *ac = *it;
			const interval_type& i = ac->get_interval();
			if(ac->is_active() || !ac->has_started() || (i.is_valid() && i.after(timestamp.second))) {
				// No need for re-calc
				// Wait all children to play
				return false;
			}
		}
	}
	// do re-calc
	return true;
}

///////////////////////////////
// seq implicit_dur

// The implicit duration of a seq ends with the active end 
// of the last child of the seq.  
// If any child of a seq has an indefinite active duration, 
// the implicit duration of the seq is also indefinite. 
time_node::time_type 
seq::get_implicit_dur() {
	if(!down()) return 0;
	const time_node* tn = last_child();
	const interval_type& i = tn->get_interval();
	time_type idur =  time_type::unresolved;
	if(i.is_valid()) idur = i.end;
	AM_DBG tnlogger->trace("%s[%s].get_implicit_dur(): %s", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), ::repr(idur).c_str());	
	return idur;
}

bool seq::needs_end_sync_update(const time_node *c, qtime_type timestamp) const {
	if(c->next()) return false;
	// c is the last
	if(!m_attrs.has_dur_specifier() && m_attrs.specified_end())
		return false;
	dur_type dt = m_attrs.get_dur_type();
	return dt == dt_unspecified || dt == dt_media;
}

///////////////////////////////
// excl implementation

excl::excl(context_type *ctx, const lib::node *n) 
:	time_container(ctx, n, tc_excl), 
	m_queue(new excl_queue()), 
	m_num_classes(0),
	m_priority_attrs(0) {
}

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
	AM_DBG tnlogger->trace("%s[%s].built_priorities()", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str());
		
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
	AM_DBG tnlogger->trace("%s[%s].built_priorities() %d classes", m_attrs.get_tag().c_str(), 
		m_attrs.get_id().c_str(), prio_classes.size());
	
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
				
				const time_attrs* pa = tn->get_time_attrs();
				AM_DBG tnlogger->trace("%s[%s] priority: %d", 
					pa->get_tag().c_str(), 
					pa->get_id().c_str(), prio);
			}
			//else not a time node
		}
		// create and store prio attrs struct 
		m_priority_attrs[prio] = priority_attrs::create_instance(priorityClassNode);
	}
}

time_node*
excl::get_active_child() {
	std::list<time_node*> cl;
	get_children(cl);
	std::list<time_node*>::iterator it;
	for(it=cl.begin();it!=cl.end();it++)
		if((*it)->is_active()) break;
	return it!=cl.end()?(*it):0;
}

void excl::interrupt(time_node *c, qtime_type timestamp) {
	time_node *interrupting_node = c;
	time_node *active_node = get_active_child();
	if(active_node == 0) {
		interrupting_node->set_state(ts_active, timestamp, this);
		return;
	}
	
	// Retrieve priority attrs of active and interrupting nodes
	priority_attrs *pi = m_priority_attrs[interrupting_node->priority()];
	priority_attrs *pa = m_priority_attrs[active_node->priority()];
	
	// get time attrs for debug printout
	const time_attrs* ta = active_node->get_time_attrs();
	const time_attrs* tai = interrupting_node->get_time_attrs();
	
	// Set interrupt attrs based on whether the interrupting node is peer, higher or lower
	interrupt_type what;
	if(interrupting_node->priority() == active_node->priority()) {
		AM_DBG tnlogger->trace("%s[%s] int by peer %s[%s]", ta->get_tag().c_str(), 
			ta->get_id().c_str(), tai->get_tag().c_str(), tai->get_id().c_str());
		what = pa->peers;
	} else if(interrupting_node->priority() > active_node->priority()) {
		AM_DBG tnlogger->trace("%s[%s] int by higher %s[%s]", ta->get_tag().c_str(), 
			ta->get_id().c_str(), tai->get_tag().c_str(), tai->get_id().c_str());
		what = pa->higher;
	} else {
		AM_DBG tnlogger->trace("%s[%s] int by lower %s[%s]", ta->get_tag().c_str(), 
			ta->get_id().c_str(), tai->get_tag().c_str(), tai->get_id().c_str());
		what = pa->lower;
	}
	
	// handle interrupt based on excl semantics
	if(what == int_stop) {
		// stop active_node
		active_node->set_state(ts_postactive, timestamp, c);
		if(ta->get_fill() == fill_freeze)
			active_node->remove(timestamp);
			
		AM_DBG tnlogger->trace("%s[%s] int_stop by %s[%s]", ta->get_tag().c_str(), 
			ta->get_id().c_str(), tai->get_tag().c_str(), tai->get_id().c_str());
				
		// start interrupting_node
		//interrupting_node->set_state(ts_active, timestamp, c);
		interrupting_node->set_state(ts_active, timestamp, c);
				
	} else if(what == int_pause) {
		// pause active_node and and insert it on the queue
		// do not cancel interval
		// a node may end while paused
		// accepts sync_update
		
		active_node->pause(timestamp, pi->display);
		m_queue->push_pause(active_node);
		
		AM_DBG tnlogger->trace("%s[%s] int_pause by %s[%s]", ta->get_tag().c_str(), 
			ta->get_id().c_str(), tai->get_tag().c_str(), tai->get_id().c_str());	
		
		// start interrupting_node
		interrupting_node->set_state(ts_active, timestamp, c);
		
	} else if(what == int_defer) {
		AM_DBG tnlogger->trace("%s[%s] continue int_defer: %s[%s]", ta->get_tag().c_str(), 
			ta->get_id().c_str(), tai->get_tag().c_str(), tai->get_id().c_str());	
		// do not touch active_node
		// defer the interrupting_node
		// arrange so that the deferred node ingnores sync_update (see spec)
		interrupting_node->defer_interval(timestamp);
		m_queue->push_defer(interrupting_node);
		
	} else { 
		// what == int_never
		// do not touch active_node
		// ignore interrupting_node
		AM_DBG tnlogger->trace("%s[%s] int_never ignore: %s[%s]", ta->get_tag().c_str(), 
			ta->get_id().c_str(), tai->get_tag().c_str(), tai->get_id().c_str());
		
		// assert that the interval is canceled
		// e.g. cancel notification to dependents
		interrupting_node->cancel_interval(timestamp);
		interrupting_node->set_state(ts_postactive, timestamp, c);
	}
}

void excl::on_child_normal_end(time_node *c, qtime_type timestamp) {
	if(!m_queue || m_queue->empty()) return;
	time_node *next = m_queue->pop();
	
	const time_attrs* ta = next->get_time_attrs();	
	AM_DBG tnlogger->trace("%s[%s].dequeue %s", ta->get_tag().c_str(), 
			ta->get_id().c_str(), (next->is_active()?"active":"deferred"));	
	
	// if its active resume else start
	if(next->paused()) {
		next->resume(timestamp);
	} else if(next->deferred()) { 
		// When an element is deferred, the begin time is deferred as well
		next->schedule_deferred_interval(timestamp);
	} else {
		assert(false);
	}
}
void excl::remove(time_node *tn) {
	if(!m_queue || m_queue->empty()) return;
	m_queue->remove(tn);
}

///////////////////////
// excl_queue

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

////////////////////////
// Debug 

#ifndef AMBULANT_NO_IOSTREAMS
void time_node::dump(std::ostream& os) {
	iterator it;
	iterator endit = end();
	int depth = 0;
	for(it = begin(); it != endit; it++) {
		if(!(*it).first) {depth--;continue;}
		time_node *tn = (*it).second;
		for(int i=0;i<depth;i++) os << "  ";
		os << tn->to_string() << " ";
		if(tn->get_interval().is_valid()) {
			os << ::repr(tn->get_interval());
		} else
			os << "[]";
		os << std::endl;	
		depth++;
	}
}

void time_node::dump_dependents(std::ostream& os, sync_event ev) {
	rule_list *p = m_dependents[ev];
	if(!p) {
		os << "dependents[" << to_string() << "] = {}" << std::endl;
		return;
	}
	os << "dependents[" << to_string() << "] = {" << std::endl;
	for(rule_list::iterator it=p->begin();it!=p->end();it++)
		os << "\t" << (*it)->to_string() << std::endl;
	os << "}" << std::endl;
}
#endif // AMBULANT_NO_IOSTREAMS
